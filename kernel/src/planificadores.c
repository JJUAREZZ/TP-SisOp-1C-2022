#include "../include/planificadores.h"
#include "../../shared/include/estructuras.h"

// ***************** Planificador a Corto Plazo ******************* 

void *planificadorCorto(t_queue* listaReady, t_log* unLogger){

	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

	if(utilizarFifo == 0){
			//Ejecutar FIFO.
			log_info(unLogger, "Planificador por FIFO.");
			planificadorFifo(listaReady, unLogger);
	}	
	if(utilizarSrt == 0){
			//hilo que ejecuta SRT.
			log_info(unLogger, "Planificador por SRT.");
			planificadorSrt(listaReady, unLogger);
	}    
}

void calcularEstimacionPcb(pcb* proceso){
	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa); 
}

void planificadorSrt(t_queue* listaReady, t_log* unLogger){
	//Falta Sincronizar.
	//Hacer que cada vez que inicia saque el de CPU.
	
	int tamanioReady = list_size(listaReady);

	while(tamanioReady > 0){

	list_iterate(listaReady, calcularEstimacionPcb);

	pcb* primerElemento;
	pcb* segundoElemento;
	int i;

	int conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);

	//Ordeno los elementos por su estimacion_actual.
	for(i=0; i<=tamanioReady; i++){
		primerElemento 	= list_get(listaReady, i);
		segundoElemento = list_get(listaReady, i+1);

		if(primerElemento->estimacion_rafaga_actual < segundoElemento->estimacion_rafaga_actual){
			list_replace(listaReady, i, segundoElemento);
			list_replace(listaReady, i + 1, primerElemento);
		} else {
			list_replace(listaReady, i, primerElemento);
			list_replace(listaReady, i+1, segundoElemento);
		}
	}

	//Envio los Procesos al CPU.
	//Ver como enviar de a uno
	pcb* elemEjecutar = queue_pop(listaReady);

	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	log_info(unLogger, "Proceso enviado a CPU");

	}	
}

//ver si tiene que agarrar un proceso o una lista de ready.
void planificadorFifo(t_queue* listaReady, t_log* unLogger){

	int tamanioReady = list_size(listaReady);
	int conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);

	while(tamanioReady > 0){

	//Enviar Primer elemento de la lista a Cpu Dispatch
	//Ver como enviar de a uno

	pcb* elemEjecutar = queue_pop(listaReady);

	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	log_info(unLogger, "Proceso enviado a CPU");

	//Recibir de blocked los Procesos.
	pcb* elemRecib; 

	}
}


//*****************************planificador a largo plazo******************************

void *atenderProceso(uint32_t accepted_fd)
{
	t_proceso *nuevoProceso= recibir_proceso(accepted_fd);
	pcb* nuevoPcb= crearPcb(nuevoProceso);
	pthread_mutex_lock(&COLANEW);
	queue_push(estadoNew,nuevoPcb);
	pthread_mutex_unlock(&COLANEW);
}

t_proceso* recibir_proceso(uint32_t accepted_fd){
	t_proceso* proceso= malloc(sizeof(t_proceso));
	uint32_t cod_op= recibir_operacion(accepted_fd);
	if(cod_op>0){
		switch (cod_op)
		{
		case PAQUETE:
			proceso = recibir_paquete(accepted_fd);
			printf("Me llego el siguiente proceso:\n");
			printf("Tamanio del Proceso en bytes: %d", proceso->tamanio);
			printf("\nInstrucciones : \n");
			void mostrarInstrucciones(instr_t* element)
			{
				printf("%s ",element->id);
				for(int i=0; i<element->nroDeParam;i++)
					printf(" %d",(int) element->param[i]);
				printf("\n");
			}
			list_iterate(proceso->instrucciones, mostrarInstrucciones);
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	return proceso;
}		

pcb *crearPcb(t_proceso *proceso)
{
	pcb *pcbDelProceso= malloc(sizeof(pcb));
	pthread_mutex_lock(&NRODEPROCESO);
	pcbDelProceso->id = nro_proceso;
	nro_proceso ++;
	pthread_mutex_unlock(&NRODEPROCESO);
	pcbDelProceso->tamanioProceso= proceso->tamanio;
	pcbDelProceso->instr= proceso->instrucciones;
	pcbDelProceso->programCounter = 0;
	pcbDelProceso->tablaDePaginas = 0;
	pcbDelProceso->estimacion_rafaga_actual = valores_generales->est_inicial;
	pcbDelProceso->estimacion_rafaga_anterior = valores_generales->est_inicial;
	pcbDelProceso->cpu_anterior = 1;
	free(proceso);
	printf("\nPCB del proceso creado");
	return pcbDelProceso;
}


void planificadorALargoPlazo()
 {
	 for(;;)
	 {
		 //falta checkear pcbs a finalizar
		pthread_mutex_lock(&COLAREADY);
		pthread_mutex_lock(&COLAEXEC);
		pthread_mutex_lock(&COLABLOCK);
		uint32_t gradoDeMultiProgActual= queue_size(estadoBlock)+
										 queue_size(estadoReady)+
										 queue_size(estadoExec);
		if(gradoDeMultiProgActual < valores_generales->grad_multiprog)
		 {	
			pthread_mutex_lock(&COLANEW);
			pcb *procesoAReady = queue_pop(estadoNew);
			pthread_mutex_unlock(&COLANEW); 

			uint32_t tablaDePaginas= obtenerTablaDePagina(procesoAReady); //TODO desarrollar
			//wait()
			if(tablaDePaginas <0){
				perror("Error al asignar memoria al proceso");
				return EXIT_FAILURE;
			}
			procesoAReady->tablaDePaginas= tablaDePaginas;
			queue_push(estadoReady, procesoAReady);
		 }
		pthread_mutex_unlock(&COLAREADY);
		pthread_mutex_unlock(&COLAEXEC);
		pthread_mutex_unlock(&COLABLOCK);
	 }
 }

 uint32_t obtenerTablaDePagina(pcb * proceso)
 {
	
	 return 1;
 }