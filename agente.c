#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define SERVER_IP "10.0.2.15"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void recolectar_metricas(char *buffer) {
    FILE *fp;
    char temp[128];

    // Uso de CPU
    fp = popen("top -bn1 | grep 'Cpu(s)' | awk '{print $2+$4}'", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "CPU: %s%%\n", temp);
    pclose(fp);

    // Uso de memoria RAM
    fp = popen("free -m | awk 'NR==2{printf \"RAM: Usada: %sMB Libre: %sMB\", $3,$4}'", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "%s\n", temp);
    pclose(fp);

    // Espacio en disco
    fp = popen("df -h --total | awk '$1 == \"total\" {print $3\"/\"$2\" Usado: \"$5}'", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "Disco: %s\n", temp);
    pclose(fp);

    // Carga promedio
    fp = popen("uptime | awk -F'load average:' '{print $2}' | cut -d, -f1", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "Carga promedio: %s\n", temp);
    pclose(fp);

    // Uso de red
    fp = popen("cat /proc/net/dev | awk 'NR>2 {print $1, $2}' | head -n 1", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "Red: %s\n", temp);
    pclose(fp);

    // Procesos en ejecución
    fp = popen("ps ax | wc -l", "r");
    fgets(temp, sizeof(temp), fp);
    snprintf(buffer + strlen(buffer), BUFFER_SIZE - strlen(buffer), "Procesos: %s\n", temp);
    pclose(fp);
}

void* enviar_metricas(void* arg) {
    int sockfd = *(int*)arg;
    char buffer[BUFFER_SIZE] = "";

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recolectar_metricas(buffer);
        send(sockfd, buffer, strlen(buffer), 0);
        sleep(5);  // Enviar métricas cada 5 segundos
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar al servidor");
        exit(1);
    }

    printf("Conectado al servidor.\n");

    pthread_t thread;
    pthread_create(&thread, NULL, enviar_metricas, &sockfd);
    pthread_join(thread, NULL);

    close(sockfd);
    return 0;
}
