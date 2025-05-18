// memoria.h
#ifndef MEMORIA_H
#define MEMORIA_H

#include <sys/types.h>

#define TAM_MEMORIA 100
#define SEMKEY 0x1234 // Clave única para semáforo
#define SHMKEY 0x5678 // Clave única para memoria compartida

typedef struct {
    int ocupado;
    pid_t pid; // PID del proceso que ocupa esta línea
} LineaMemoria;

typedef struct {
    LineaMemoria lineas[TAM_MEMORIA];
    int procesos_activos;
} MemoriaCompartida;

#endif