#include "../include/cpu.h"
#include "../include/utils.h"
#include "sys/time.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/time.h> 

struct timeval t1, t2;
double computarTiempo();
uint32_t check_interrupt();

int main() {

    load_configuration_cpu();
    conectar_dispatcher();

    return 0;    
}

void atenderInterrupcion(uint32_t accepted_fd){
	while(1){
		uint32_t cod_op= recibir_operacion(accepted_fd);
		if(cod_op>0){
			switch (cod_op){
			case DESALOJARPROCESO:
				interrupcion= 1;
				break;
			default:
				;
			}
		}
	}	
}

void *atenderPcb(uint32_t accepted_fd){	

	while(1){
		uint32_t cod_op= recibir_operacion(accepted_fd);
		if(cod_op>0)
		{
			switch (cod_op)
			{
			case PCB:
		
				unPcb= recibir_pcb(accepted_fd);
				log_info(logger,"Recibi un proceso:");
				log_info(logger,"id: %d",unPcb->id);
				//printf("\ntamanioProceso: %d",unPcb->tamanioProceso);
				//printf("\nprogramCounter: %d", unPcb->programCounter);
				//printf("\ntablaDePaginas: %d",unPcb->tablaDePaginas);
				//printf("\nestimacion_rafaga_actual: %d",unPcb->estimacion_rafaga_actual);
				//printf("\nestimacion_rafaga_anterior: %d",unPcb->estimacion_rafaga_anterior);
				//printf("\ncpu_anterior: %f\n",unPcb->cpu_anterior);
				
				void mostrarInstrucciones(instr_t* element)
				{
					printf("%s ",element->id);
					for(int i=0; i<element->nroDeParam;i++)
						printf(" %d",(int) element->param[i]);
					printf("\n");
				}
				//list_iterate(unPcb->instr, mostrarInstrucciones);
				ciclo_de_instruccion(accepted_fd);
	
				break;
			default:
				break;
			}
		}
	}		
	
}
	
ciclo_de_instruccion(uint32_t accepted_fd){

	struct timeval initialBlock;
	struct timeval finalBlock;

	uint32_t cpu_pasado;

	/*while(check_interrupt){*/
	while(1){
		gettimeofday(&initialBlock, NULL);
		// FETCH
		instr_t* instruccion;
		instruccion = list_get(unPcb->instr, unPcb->programCounter);
		
		//DECODE
		char* nombreInstruccion = instruccion->id;
		log_info(logger,nombreInstruccion);
		
		//EXECUTE
		if(strcmp(nombreInstruccion, "NO_OP") == 0){

			log_info(logger,"Esperando %d milisegundos",cpu_config->retar_noop*instruccion->param[0]);
			usleep(cpu_config->retar_noop*instruccion->param[0]*1000);  
			//lo multiplico por 1000 porque usleep recibe el tiempo en microsegundos
			gettimeofday(&finalBlock, NULL);
			cpu_pasado = time_diff(&initialBlock, &finalBlock);
			ceil(cpu_pasado);
			unPcb->cpu_anterior+= cpu_pasado;
			
		} else if(strcmp(nombreInstruccion, "I/O") == 0){
			gettimeofday(&finalBlock, NULL);
			cpu_pasado = time_diff(&initialBlock, &finalBlock);
			ceil(cpu_pasado);

			log_info(logger,"El tiempo de ejecucion fue : %d", cpu_pasado);

			unPcb->cpu_anterior+= cpu_pasado;
			devolverPcb(BLOCKED, accepted_fd);
			log_info(logger,"Proceso %d enviado a bloqueado.", unPcb->id);
			return;

		} else if(strcmp(nombreInstruccion, "READ") == 0){

		} else if(strcmp(nombreInstruccion, "WRITE") == 0){

		} else if(strcmp(nombreInstruccion, "COPY") == 0) {

		} else if(strcmp(nombreInstruccion, "EXIT") == 0) {
			gettimeofday(&finalBlock, NULL); //es necesario computar el tiempo en exit?
			log_info(logger,"Proceso %d enviado a exit.", unPcb->id);
			devolverPcb(PROCESOTERMINATED, accepted_fd);
			
			return;

		}
		unPcb->programCounter+=1;
		//CHECK INTERRUPT 
		if(check_interrupt()==1)
			return;
	}

} 


uint32_t check_interrupt(){
	if(interrupcion ==1){
		interrupcion =0;
		log_info(logger,"Se ha recibido una interrupcion del Kernel");
		log_info(logger,"Desalojando proceso: %d", unPcb->id);
		t_paquete *paquete= crear_paquete(PROCESODESALOJADO);
		agregarPcbAPaquete(paquete,unPcb);
		enviar_paquete(paquete, socket_dispatch);
		eliminar_paquete(paquete);
		liberarPcb(unPcb);
		return 1;
	}
	return 0;
}

void devolverPcb(uint32_t co_op, uint32_t accepted_fd){
	t_paquete *paquete = crear_paquete(co_op);
	agregarPcbAPaquete(paquete, unPcb);
	enviar_paquete(paquete, accepted_fd);
	eliminar_paquete(paquete);
	liberarPcb(unPcb);
}

//*** UTILIZAR sem_post(&semEnviarDispatch); CUANDO LA CPU ESTE DESOCUPADA ***
//*** CUANDO interrumpirCPU = 1, interrumpir la CPU y enviar el PCB a KERNEL *** 