#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include "memoria.h"

// Semáforo (operaciones)
struct sembuf sem_lock = {0, -1, 0};  // P
struct sembuf sem_unlock = {0, 1, 0}; // V

const int FIRST_FIT = 1;
const int BEST_FIT = 2;
const int WORST_FIT = 3;

// Algoritmo de asignación global
int algoritmo = 0; // 1: First-Fit, 2: Best-Fit, 3: Worst-Fit

// Referencias a recursos compartidos
MemoriaCompartida *memoria = NULL;
int semid;

// Función para registrar en bitácora
void registrar_bitacora(pid_t pid, const char *accion, int inicio, int cantidad) {
    FILE *f = fopen("bitacora.txt", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char hora[26];
        strftime(hora, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(f, "[%s] PID %d - %s - líneas %d a %d\n", hora, pid, accion, inicio, inicio + cantidad - 1);
        fclose(f);
    }
}

// Algoritmos de asignación
int buscar_memoria(int size) {
    int mejor_inicio = -1;
    int mejor_tamano = (algoritmo == BEST_FIT) ? TAM_MEMORIA + 1 : -1;

    int i = 0;
    // TODO: Implementar el algoritmo de asignación
    while (i < TAM_MEMORIA) {
        if (!memoria->lineas[i].ocupado) {
            int inicio = i;
            int libre = 0;
            while (i < TAM_MEMORIA && !memoria->lineas[i].ocupado) {  
                libre++;
                if (libre == size) break;
                i++;
            }

            if (libre >= size) { 
                if (algoritmo == FIRST_FIT) return inicio; // First-Fit
                if (algoritmo == BEST_FIT && libre < mejor_tamano) {
                    mejor_inicio = inicio;
                    mejor_tamano = libre;
                }
                if (algoritmo == WORST_FIT && libre > mejor_tamano) {
                    mejor_inicio = inicio;
                    mejor_tamano = libre;
                }
            }
        }
        i++;
    }

    return (algoritmo == FIRST_FIT) ? mejor_inicio : (mejor_tamano == TAM_MEMORIA + 1) ? -1 : mejor_inicio;
}

// Función que ejecuta cada "proceso" (hilo)
void *proceso(void *arg) {
    pid_t pid = getpid(); // mismo para todos, pero para simulación está bien
    pthread_t tid = pthread_self();
    int size = (rand() % 10) + 1;
    int duracion = (rand() % 41) + 20;

    int inicio = -1;

    semop(semid, &sem_lock, 1); // Entrar a región crítica

    inicio = buscar_memoria(size);
    if (inicio != -1) {
        for (int i = inicio; i < inicio + size; i++) {
            memoria->lineas[i].ocupado = 1;
            memoria->lineas[i].pid = tid; // Usamos tid como "PID" interno
        }
        registrar_bitacora(tid, "ASIGNACION", inicio, size);
    }

    semop(semid, &sem_unlock, 1); // Salir de región crítica

    if (inicio == -1) {
        registrar_bitacora(tid, "FALLO_ASIGNACION", 0, 0);
        pthread_exit(NULL);
    }

    sleep(duracion); // Simula la ejecución

    semop(semid, &sem_lock, 1); // Región crítica para liberar memoria

    for (int i = inicio; i < inicio + size; i++) {
        memoria->lineas[i].ocupado = 0;
        memoria->lineas[i].pid = 0;
    }
    registrar_bitacora(tid, "DESASIGNACION", inicio, size);

    semop(semid, &sem_unlock, 1);

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    printf("Seleccione algoritmo (1: First-Fit, 2: Best-Fit, 3: Worst-Fit): ");
    scanf("%d", &algoritmo);
    if (algoritmo < 1 || algoritmo > 3) {
        printf("Algoritmo inválido.\n");
        exit(1);
    }

    // Conectarse a memoria y semáforo
    int shmid = shmget(SHMKEY, sizeof(MemoriaCompartida), 0666);
    if (shmid == -1) {
        perror("Error al acceder a memoria compartida");
        exit(1);
    }
    memoria = (MemoriaCompartida *)shmat(shmid, NULL, 0);
    semid = semget(SEMKEY, 1, 0666);
    if (semid == -1) {
        perror("Error al acceder al semáforo");
        exit(1);
    }

    // Lanzar hilos aleatoriamente cada 30-60s
    while (1) {
        pthread_t hilo;
        pthread_create(&hilo, NULL, proceso, NULL);
        pthread_detach(hilo); // No esperamos que finalice

        int espera = (rand() % 31) + 30;
        sleep(espera);
    }

    return 0;
}
