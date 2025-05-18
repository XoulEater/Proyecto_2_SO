#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include "memoria.h"

// Inicializa el semáforo con valor 1
void inicializar_semaforo(int semid) {
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("Error al inicializar el semáforo");
        exit(1);
    }
}

int main() {
    int shmid, semid;
    MemoriaCompartida *memoria;

    // Crear memoria compartida
    shmid = shmget(SHMKEY, sizeof(MemoriaCompartida), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error al crear memoria compartida");
        exit(1);
    }

    // Conectar el segmento de memoria
    memoria = (MemoriaCompartida *)shmat(shmid, NULL, 0);
    if (memoria == (void *)-1) {
        perror("Error al conectar memoria compartida");
        exit(1);
    }

    // Inicializar la memoria
    for (int i = 0; i < TAM_MEMORIA; i++) {
        memoria->lineas[i].ocupado = 0;
        memoria->lineas[i].pid = 0;
    }
    memoria->procesos_activos = 0;

    // Crear semáforo
    semid = semget(SEMKEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Error al crear semáforo");
        exit(1);
    }

    inicializar_semaforo(semid);

    printf("Inicialización completada.\n");
    printf("Memoria compartida ID: %d\n", shmid);
    printf("Semáforo ID: %d\n", semid);

    // Desconectar de la memoria
    shmdt(memoria);

    return 0;
}