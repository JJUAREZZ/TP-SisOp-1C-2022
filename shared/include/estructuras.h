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
	int id;
	uint32_t tamanioProceso;
	t_list* instr;
	int programCounter;
	int tablaDePaginas;
	float estimacion_rafaga;
} pcb;

#endif