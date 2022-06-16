#include "../include/cpu.h"
#include "../include/utils.h"
#include "sys/time.h"
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

void *atenderPcb(uint32_t accepted_fd){			
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

				ciclo_de_instruccion(accepted_fd);
	
				break;
			default:
				break;
			}
		}
}
	
ciclo_de_instruccion(uint32_t accepted_fd){

	struct timeval initialBlock;
	struct timeval finalBlock;

	uint32_t cpu_pasado;

	gettimeofday(&initialBlock, NULL);
	/*while(check_interrupt){*/
	while(1){
		
		// FETCH
		instr_t* instruccion;
		instruccion = list_get(unPcb->instr, unPcb->programCounter);
		
		//DECODE
		char* nombreInstruccion = instruccion->id;
		printf(nombreInstruccion);
		
		//EXECUTE
		if(strcmp(nombreInstruccion, "NO_OP") == 0){

			printf("\nEsperando %d milisegundos\n",cpu_config->retar_noop*instruccion->param[0]);
			usleep(cpu_config->retar_noop*instruccion->param[0]);

		} else if(strcmp(nombreInstruccion, "I/O") == 0){

			gettimeofday(&finalBlock, NULL);
			cpu_pasado = time_diff(&initialBlock, &finalBlock);
			printf("El tiempon de ejecucion fue : %d", cpu_pasado);
			unPcb->cpu_anterior = cpu_pasado * 1000;
			devolverPcb(BLOCKED, accepted_fd);
			printf("\nProceso %d enviado a bloqueado\n.", unPcb->id);
			break;

		} else if(strcmp(nombreInstruccion, "READ") == 0){

		} else if(strcmp(nombreInstruccion, "WRITE") == 0){

		} else if(strcmp(nombreInstruccion, "COPY") == 0) {

		} else if(strcmp(nombreInstruccion, "EXIT") == 0) {

			gettimeofday(&finalBlock, NULL);
			devolverPcb(PROCESOTERMINATED, accepted_fd);
			printf("\nProceso %d enviado a exit.\n", unPcb->id);
			break;

		}
		unPcb->programCounter = unPcb->programCounter + 1;

	} 

}


void check_interrupt(pcb* pcb){
	return true;
}

void devolverPcb(uint32_t co_op, uint32_t accepted_fd){
	t_paquete *paquete = crear_paquete(co_op);
	agregarPcbAPaquete(paquete, unPcb);
	enviar_paquete(paquete, accepted_fd);
	eliminar_paquete(paquete);
}

//*** UTILIZAR sem_post(&semEnviarDispatch); CUANDO LA CPU ESTE DESOCUPADA ***
//*** CUANDO interrumpirCPU = 1, interrumpir la CPU y enviar el PCB a KERNEL *** 