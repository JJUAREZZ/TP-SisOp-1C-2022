#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <semaphore.h>

//Semaforos...
sem_t* semEnviarDispatch;
sem_t* semInterrumpirCPU;
sem_t* semProcesosEnReady;
sem_t* semProcesosEnRunning;
sem_t* semProcesosEnExit;

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
	float estimacion_rafaga_actual;
	float estimacion_rafaga_anterior;
	float cpu_anterior;
} pcb;

#endif /* SRC_ESTRUCTURAS_H_ */