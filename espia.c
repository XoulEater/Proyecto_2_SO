#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include "memoria.h"

// Operaciones de semáforo
struct sembuf sem_lock = {0, -1, 0};
struct sembuf sem_unlock = {0, 1, 0};

// Verifica si un proceso con cierto PID está en ejecución
int proceso_ejecutando(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d", pid);
    return access(path, F_OK) == 0;
}

// Verifica si un proceso está bloqueado
int proceso_bloqueado(pid_t pid) {
    char path[64], estado;
    FILE *f;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    f = fopen(path, "r");
    if (!f) return 0;

    while (fgets(path, sizeof(path), f)) {
        if (strncmp(path, "State:", 6) == 0) {
            sscanf(path + 6, " %c", &estado);
            fclose(f);
            return estado == 'D' || estado == 'T'; // D = sleep in uninterruptible I/O, T = stopped
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int shmid, semid;
    MemoriaCompartida *memoria;

    // Conectarse a la memoria compartida
    shmid = shmget(SHMKEY, sizeof(MemoriaCompartida), 0666);
    if (shmid == -1) {
        perror("Error al acceder a la memoria compartida");
        exit(1);
    }

    memoria = (MemoriaCompartida *)shmat(shmid, NULL, 0);
    if (memoria == (void *)-1) {
        perror("Error al conectar a la memoria compartida");
        exit(1);
    }

    // Conectarse al semáforo
    semid = semget(SEMKEY, 1, 0666);
    if (semid == -1) {
        perror("Error al acceder al semáforo");
        exit(1);
    }

    // Entrar a región crítica
    semop(semid, &sem_lock, 1);

    printf("Estado de la memoria:\n");
    for (int i = 0; i < TAM_MEMORIA; i++) {
        if (memoria->lineas[i].ocupado) {
            printf("Línea %d: Ocupada por PID %d\n", i, memoria->lineas[i].pid);
        } else {
            printf("Línea %d: Libre\n", i);
        }
    }

    printf("\nProcesos con acceso a la memoria:\n");
    int listado[1024] = {0};
    int encontrados = 0;
    for (int i = 0; i < TAM_MEMORIA; i++) {
        pid_t pid = memoria->lineas[i].pid;
        if (memoria->lineas[i].ocupado) {
            int ya_listado = 0;
            for (int j = 0; j < encontrados; j++) {
                if (listado[j] == pid) {
                    ya_listado = 1;
                    break;
                }
            }
            if (!ya_listado) {
                listado[encontrados++] = pid;
                printf("PID %d\n", pid);
            }
        }
    }

    printf("\nEstado de los procesos:\n");
    for (int i = 0; i < encontrados; i++) {
        pid_t pid = listado[i];
        if (proceso_bloqueado(pid)) {
            printf("PID %d: BLOQUEADO\n", pid);
        } else if (proceso_ejecutando(pid)) {
            printf("PID %d: EJECUTANDO\n", pid);
        } else {
            printf("PID %d: TERMINADO o ZOMBIE\n", pid);
        }
    }

    // Salir de región crítica
    semop(semid, &sem_unlock, 1);

    // Desconectarse de la memoria
    shmdt(memoria);

    return 0;
}
