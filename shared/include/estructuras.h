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
	float estimacion_rafaga_actual;
	float estimacion_rafaga_anterior;
	float cpu_anterior;
} pcb;

void paquete_pcb(pcb *proceso, int conexion){
	t_paquete *paquete= crear_paquete();
	agregar_a_paquete(paquete,proceso);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

#endif