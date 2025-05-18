# Compilador
CC = gcc

# Flags comunes (hilos, warnings, estándar C99)
CFLAGS = -Wall -pthread -std=c99

# Objetivos
all: inicializador productor espia finalizador

inicializador: inicializador.c
	$(CC) $(CFLAGS) -o inicializador inicializador.c

productor: productor.c
	$(CC) $(CFLAGS) -o productor productor.c

# espia: espia.c
# 	$(CC) $(CFLAGS) -o espia espia.c

finalizador: finalizador.c
	$(CC) $(CFLAGS) -o finalizador finalizador.c

# Limpiar ejecutables
clean:
	rm -f inicializador productor espia finalizador bitacora.txt
