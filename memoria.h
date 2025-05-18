#define TAM_MEMORIA 100

typedef struct {
    int ocupado;
    pid_t pid; // PID del proceso que ocupa esta línea, si hay uno
} LineaMemoria;

typedef struct {
    LineaMemoria lineas[TAM_MEMORIA];
    int procesos_activos;
} MemoriaCompartida;