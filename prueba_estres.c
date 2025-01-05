#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_NUM_THREADS 100
#define DEFAULT_LOAD_DURATION 10  // En segundos

typedef struct {
    int thread_id;
    int duration;
} ThreadArgs;

void* generate_load(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int id = args->thread_id;
    int duration = args->duration;

    printf("[Hilo %d] Iniciando carga durante %d segundos...\n", id, duration);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        // Simulación de carga: cálculos intensivos
        double result = 0.0;
        for (int i = 0; i < 1000000; i++) {
            result += i * 0.1;  // Cálculo ficticio
        }
    }

    printf("[Hilo %d] Carga completada.\n", id);
    free(args);  // Liberar memoria de los argumentos
    return NULL;
}

int main(int argc, char* argv[]) {
    int num_threads = DEFAULT_NUM_THREADS;
    int duration = DEFAULT_LOAD_DURATION;

    // Leer argumentos de línea de comandos (opcional)
    if (argc >= 2) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0) {
            fprintf(stderr, "El número de hilos debe ser mayor que 0.\n");
            return 1;
        }
    }
    if (argc >= 3) {
        duration = atoi(argv[2]);
        if (duration <= 0) {
            fprintf(stderr, "La duración debe ser mayor que 0.\n");
            return 1;
        }
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
    if (!threads) {
        perror("Error al asignar memoria para los hilos");
        return 1;
    }

    printf("[INFO] Iniciando prueba de estrés con %d hilos durante %d segundos...\n", num_threads, duration);

    for (int i = 0; i < num_threads; i++) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        if (!args) {
            perror("Error al asignar memoria para los argumentos del hilo");
            free(threads);
            return 1;
        }
        args->thread_id = i;
        args->duration = duration;

        if (pthread_create(&threads[i], NULL, generate_load, args) != 0) {
            perror("[ERROR] Error al crear el hilo");
            free(args);
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[INFO] Prueba de estrés completada.\n");

    free(threads);
    return 0;
}
