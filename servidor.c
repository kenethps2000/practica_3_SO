#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void enviar_alerta_twilio(const char *mensaje) {
    char comando[512];
    snprintf(comando, sizeof(comando), "./enviar_alerta whatsapp:+593981773913 \"%s\"", mensaje);
    system(comando);
}

void evaluar_metricas(const char *buffer) {
    float cpu_usage = 0.0;
    int ram_libre = 0;

    // Buscar y extraer el valor de CPU
    const char *cpu_ptr = strstr(buffer, "CPU: ");
    if (cpu_ptr) {
        cpu_ptr += 5; // Avanzar el puntero después de "CPU: "
        char *end_ptr;
        cpu_usage = strtof(cpu_ptr, &end_ptr);
        if (*end_ptr == ',') {
            // Convertir coma decimal a punto decimal
            cpu_usage = strtof(cpu_ptr, NULL);
        }
    }

    // Buscar y extraer el valor de RAM libre
    const char *ram_ptr = strstr(buffer, "RAM: Usada:");
    if (ram_ptr) {
        ram_ptr = strstr(ram_ptr, "Libre: ");
        if (ram_ptr) {
            ram_ptr += 7; // Avanzar el puntero después de "Libre: "
            ram_libre = atoi(ram_ptr);
        }
    }

    // Evaluar las métricas y enviar alertas si se superan los umbrales
    if (cpu_usage > 50.0) {
        enviar_alerta_twilio("CPU supera el 50%");
    }
    if (ram_libre < 500) { // Umbral de ejemplo: menos de 500MB libres
        enviar_alerta_twilio("RAM disponible menor a 500MB");
    }
}

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    FILE* dashboard = fopen("dashboard.txt", "a");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        fprintf(dashboard, "%s\n", buffer);
        fflush(dashboard);

        evaluar_metricas(buffer);
    }

    fclose(dashboard);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    printf("Servidor en espera de conexiones...\n");

    while (1) {
        int* client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, NULL, NULL);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}
