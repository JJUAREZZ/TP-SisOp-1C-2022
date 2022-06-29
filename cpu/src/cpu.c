#include "../include/cpu.h"
#include "../include/utils.h"
#include "sys/time.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/time.h> 

uint32_t check_interrupt();
pthread_t conexion_con_memoria;
uint32_t mmu (uint32_t , uint32_t );

int main() {

    load_configuration_cpu();
	pthread_create(&conexion_con_memoria, NULL, conectarse_con_memoria, NULL);
	pthread_detach(conexion_con_memoria);
    conectar_dispatcher();

    return 0;    
}

void atenderInterrupcion(uint32_t accepted_fd){
	while(1){
		uint32_t cod_op= recibir_operacion(accepted_fd);
		if(cod_op>0){
			switch (cod_op){
			case DESALOJARPROCESO:
				
				if(unPcb==NULL){
					uint32_t rta= CPUVACIA;
					send(socket_dispatch, &rta, sizeof(uint32_t), 0);
				}
				else{
					interrupcion= 1;
				}
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

			log_info(logger,"Esperando %d milisegundos",cpu_config->retar_noop);
			usleep(cpu_config->retar_noop*1000);  
			//lo multiplico por 1000 porque usleep recibe el tiempo en microsegundos
			gettimeofday(&finalBlock, NULL);
			cpu_pasado = time_diff(&initialBlock, &finalBlock);
			unPcb->cpu_anterior+= cpu_pasado;

		} else if(strcmp(nombreInstruccion, "I/O") == 0){
			gettimeofday(&finalBlock, NULL);
			cpu_pasado = time_diff(&initialBlock, &finalBlock);
			unPcb->cpu_anterior+= cpu_pasado;
			log_info(logger,"El tiempo de ejecucion fue : %d", unPcb->cpu_anterior);

			log_info(logger,"Proceso %d enviado a bloqueado.", unPcb->id);
			devolverPcb(BLOCKED, accepted_fd);
			
			interrupcion =0; //no checkea interrupciones, así que la pone en 0
			return;

		} else if(strcmp(nombreInstruccion, "READ") == 0){
			uint32_t direccionLogica= instruccion->param[0];
			uint32_t tablaDePaginas= unPcb->tablaDePaginas;
			
			uint32_t direccionFisica= mmu(direccionLogica,tablaDePaginas);
			//aca se conecta a memoria, para pasarle la direccion fisica con el cod_op READ

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
		unPcb=NULL;
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
	unPcb=NULL;
}

uint32_t mmu (uint32_t direccion_logica, uint32_t tablaDePaginas){
	uint32_t numero_pagina,entrada_tabla_1er_nivel,entrada_tabla_2do_nivel,desplazamiento;

	numero_pagina= floor(direccion_logica / memoria_config->tam_pagina);
	entrada_tabla_1er_nivel = floor(numero_pagina / memoria_config->entradas_por_tabla);
	entrada_tabla_2do_nivel = numero_pagina % memoria_config->entradas_por_tabla;
    desplazamiento = direccion_logica - (numero_pagina * memoria_config->tam_pagina);
	//printf("%d\n", memoria_config->tam_pagina);

//FALTA TLB

	uint32_t id_tabla_primer_nivel, id_tabla_segundo_nivel, marco, direccion_fisica;
	uint32_t cod_op, offset, tamanio;
	void *buffer;

	id_tabla_primer_nivel= tablaDePaginas;

	//enviar paquete con id_tabla_primer_nivel y entrada_tabla_1er_nivel
	cod_op= IDTABLASEGUNDONIVEL;
	offset=0;
	tamanio= sizeof(uint32_t)*3;
	buffer= malloc(tamanio);
	memcpy(buffer+offset,&cod_op,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&id_tabla_primer_nivel,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&entrada_tabla_1er_nivel,sizeof(uint32_t));
	send(socket_memoria, buffer, tamanio, 0);
	free(buffer);
	recv(socket_memoria, &id_tabla_segundo_nivel, sizeof(uint32_t), MSG_WAITALL);
	
	//enviar paquete con id_tabla_segundo_nivel y entrada_tabla_2do_nivel
	cod_op= MARCO;
	offset=0;
	tamanio= sizeof(uint32_t)*3;
	buffer= malloc(tamanio);
	memcpy(buffer+offset,&cod_op,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&id_tabla_segundo_nivel,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&entrada_tabla_2do_nivel,sizeof(uint32_t));
	send(socket_memoria, buffer, tamanio, 0);
	free(buffer);
	recv(socket_memoria, &marco, sizeof(uint32_t), MSG_WAITALL);

	direccion_fisica= (marco * memoria_config->tam_pagina) + desplazamiento;

	return direccion_fisica;
}

