#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include "memoria.h"

int main() {
    int shmid, semid;
    MemoriaCompartida *memoria;

    // Conectarse a la memoria compartida
    shmid = shmget(SHMKEY, sizeof(MemoriaCompartida), 0666);
    if (shmid == -1) {
        perror("Error al acceder a memoria compartida");
        exit(1);
    }

    memoria = (MemoriaCompartida *)shmat(shmid, NULL, 0);
    if (memoria == (void *)-1) {
        perror("Error al conectar memoria compartida");
        exit(1);
    }

    // Conectarse al semáforo
    semid = semget(SEMKEY, 1, 0666);
    if (semid == -1) {
        perror("Error al acceder al semáforo");
        exit(1);
    }

    printf("Limpieza de recursos...\n");

    // Eliminar memoria compartida
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la memoria compartida");
    } else {
        printf("Memoria compartida eliminada.\n");
    }

    // Eliminar semáforo
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Error al eliminar el semáforo");
    } else {
        printf("Semáforo eliminado.\n");
    }

    // Desconectarse de la memoria
    shmdt(memoria);

    // Registrar en bitácora
    FILE *bitacora = fopen("bitacora.txt", "a");
    if (bitacora) {
        fprintf(bitacora, "[Finalizador] Recursos eliminados. Hilos deben ser finalizados por el Productor.\n");
        fclose(bitacora);
    }

    printf("Finalización completa.\n");
    return 0;
}
