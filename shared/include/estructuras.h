#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

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


#endif