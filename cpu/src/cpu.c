#include "../include/cpu.h"
#include "../include/utils.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>



int main() {

    load_configuration_cpu();
    conectar_dispatcher();

    return 0;    
}

void atenderInterrupcion(uint32_t accepted_fd){

}

void *atenderPcb(uint32_t accepted_fd)
{
	pcb *unPcb;
	while(1){
	uint32_t cod_op= recibir_operacion(accepted_fd);
		if(cod_op>0)
		{
			switch (cod_op)
			{
			case PCB:
		
				unPcb= recibir_pcb(accepted_fd);
				printf("\nRecibi un proceso:");
				printf("\nid: %d",unPcb->id);
				printf("\ntamanioProceso: %d",unPcb->tamanioProceso);
				printf("\nprogramCounter: %d", unPcb->programCounter);
				printf("\ntablaDePaginas: %d",unPcb->tablaDePaginas);
				printf("\nestimacion_rafaga_actual: %d",unPcb->estimacion_rafaga_actual);
				printf("\nestimacion_rafaga_anterior: %d",unPcb->estimacion_rafaga_anterior);
				printf("\ncpu_anterior: %f\n",unPcb->cpu_anterior);
				void mostrarInstrucciones(instr_t* element)
				{
					printf("%s ",element->id);
					for(int i=0; i<element->nroDeParam;i++)
						printf(" %d",(int) element->param[i]);
					printf("\n");
				}
				list_iterate(unPcb->instr, mostrarInstrucciones);
			break;
			default:
				;
			}
		}

	}

	//ciclo_de_instruccion(nuevoPcb);
}

void ciclo_de_instruccion(pcb* pcb) {
	
	// FETCH
	instr_t* instruccion;
	instruccion = list_get(pcb->instr, pcb->programCounter);
	
	//DECODE
	char* nombreInstruccion = instruccion->id;
	printf(nombreInstruccion);

	//EXECUTE
	if(strcmp(nombreInstruccion, "NO_OP") == 0){
		usleep((cpu_config->retar_noop*instruccion->param[0]) * 100);
		check_interrupt(pcb);
	} else if(strcmp(nombreInstruccion, "I/O") == 0){

	} else if(strcmp(nombreInstruccion, "READ") == 0){

	} else if(strcmp(nombreInstruccion, "WRITE") == 0){

	} else if(strcmp(nombreInstruccion, "COPY") == 0) {

	} else if(strcmp(nombreInstruccion, "EXIT") == 0) {

	}
}

void check_interrupt(pcb* pcb){

}

//*** UTILIZAR sem_post(&semEnviarDispatch); CUANDO LA CPU ESTE DESOCUPADA ***
//*** CUANDO interrumpirCPU = 1, interrumpir la CPU y enviar el PCB a KERNEL *** 