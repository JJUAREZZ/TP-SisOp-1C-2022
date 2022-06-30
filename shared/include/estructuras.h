#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <semaphore.h>
#include <stdint.h>
#include <commons/collections/list.h>

//Semaforos...
sem_t semEnviarDispatch;
sem_t semProcesoInterrumpido;
sem_t semProcesosEnReady;
sem_t semProcesosEnBlock;
sem_t semProcesosEnSuspReady;
sem_t semProcesosEnRunning;
sem_t semProcesosEnExit;
sem_t semProcesosEnNew;
sem_t semProcesosOrdenados;
sem_t semProcesoCpu;
sem_t semSrt;
sem_t sem_obtener_tabla_de_paginas;
sem_t sem_proceso_suspendido;
sem_t sem_swap_proceso_terminado;

//Flag para interrumpir la cpu.
uint32_t *interrumpirCPU;

typedef struct {
	int idLength;
	char* id;
	int nroDeParam;
	int *param;
} instr_t;

//Estructura del pcb
typedef struct{
	uint32_t id;
	uint32_t tamanioProceso;
	t_list* instr;
	uint32_t programCounter;
	uint32_t tablaDePaginas;
	int estimacion_rafaga_actual;
	int estimacion_rafaga_anterior;
	int cpu_anterior;
} pcb;

void liberarPcb(pcb*);

#endif /* SRC_ESTRUCTURAS_H_ */