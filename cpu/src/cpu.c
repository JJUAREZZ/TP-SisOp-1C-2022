#include "../include/cpu.h"
#include "../include/utils.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/time.h> 

struct timeval t1, t2;
double computarTiempo();
uint32_t check_interrupt(pcb* proceso);

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
				log_info(logger, "Se ha recibido una interrupcion del Kernel");
				interrupcion= 1;
				break;
			default:
				;
			}
		}
	}	
}

void *atenderPcb(uint32_t accepted_fd)
{
	pcb *unPcb;
	while(1){
	uint32_t cod_op= recibir_operacion(accepted_fd);
	interrupcion=0;
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
				ciclo_de_instruccion(unPcb);
			break;
			default:
				;
			}
		}

	}

	
}

void ciclo_de_instruccion(pcb* proceso) {

	gettimeofday(&t1, NULL); //se inicia el temporizador
	log_info(logger, "Ejecutando instrucciones:");
	while(1){
		// FETCH
		instr_t* instruccion;
		instruccion = list_get(proceso->instr, proceso->programCounter);
		
		//DECODE
		char* nombreInstruccion = instruccion->id;
		log_info(logger,"%s",nombreInstruccion);

		//EXECUTE
		if(strcmp(nombreInstruccion, "NO_OP") == 0){
			usleep((cpu_config->retar_noop*instruccion->param[0]) * 1000);
		} else if(strcmp(nombreInstruccion, "I/O") == 0){
			
		} else if(strcmp(nombreInstruccion, "READ") == 0){

		} else if(strcmp(nombreInstruccion, "WRITE") == 0){

		} else if(strcmp(nombreInstruccion, "COPY") == 0) {

		} else if(strcmp(nombreInstruccion, "EXIT") == 0) {
			t_paquete *paquete= crear_paquete(PROCESOTERMINATED);
			agregarPcbAPaquete(paquete,proceso);
			enviar_paquete(paquete, socket_dispatch);
			eliminar_paquete(paquete);
			liberarPcb(proceso);
			return;
		}
		gettimeofday(&t2, NULL);
		
		proceso->cpu_anterior+= computarTiempo();
		proceso->programCounter+=1;

		//CHECK INTERRUPT 
		if(check_interrupt(proceso)==1)
			return;
	}
	
}

uint32_t check_interrupt(pcb* proceso){
	if(interrupcion ==1){
		log_info(logger,"Desalojando proceso: %d", proceso->id);
		t_paquete *paquete= crear_paquete(PROCESODESALOJADO);
		agregarPcbAPaquete(paquete,proceso);
		enviar_paquete(paquete, socket_dispatch);
		eliminar_paquete(paquete);
		liberarPcb(proceso);
		return 1;
	}
	return 0;
}

double computarTiempo(){
    double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;  
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   
	return elapsedTime;
}

//*** UTILIZAR sem_post(&semEnviarDispatch); CUANDO LA CPU ESTE DESOCUPADA ***
//*** CUANDO interrumpirCPU = 1, interrumpir la CPU y enviar el PCB a KERNEL *** 