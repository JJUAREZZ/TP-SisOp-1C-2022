#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <semaphore.h>
#include <stdint.h>
#include <commons/collections/list.h>

//Semaforos...
sem_t semEnviarDispatch;
sem_t semInterrumpirCPU;
sem_t semProcesosEnReady;
sem_t semProcesoReadySrt;
sem_t semProcesosEnRunning;
sem_t semProcesosEnExit;
sem_t semProcesosEnNew;

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
	uint32_t estimacion_rafaga_actual;
	uint32_t estimacion_rafaga_anterior;
	uint32_t cpu_anterior;
} pcb;

void liberarPcb(pcb*);

#endif /* SRC_ESTRUCTURAS_H_ */