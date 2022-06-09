#include "../include/planificadores.h"
#include "../../shared/include/estructuras.h"
#include "sys/time.h"
#include <unistd.h>

//*****************************planificador a corto plazo****************************

void *planificadorACortoPlazo(){
	
	u_int32_t conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);

	if(conexion_cpu_dispatch < 0){
		printf("Fallo a conectarse con cpu_dispatch");
		return EXIT_FAILURE;
	}

	u_int32_t conexion_cpu_interrupt = socket_connect_to_server(config_valores_cpu_interrupt->ip, config_valores_cpu_interrupt->puerto);

	if(conexion_cpu_interrupt < 0){
		printf("Fallo a conectarse con cpu_interrupt");
		return EXIT_FAILURE;
	}

	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

	pthread_mutex_lock(&COLABLOCKREADY);

	uint32_t tamanioReady = queue_size(estadoReady);
	while(tamanioReady > 0){

	if(utilizarFifo == 0){
			//Ejecutar FIFO.
			planificadorFifo(conexion_cpu_dispatch);
			printf("Planificador por FIFO.");
	}

	if(utilizarSrt == 0){
			//hilo que ejecuta SRT.
			planificadorSrt(conexion_cpu_interrupt);
			printf ("Planificador por SRT.");
	}   

	}
	pthread_mutex_unlock(&COLABLOCKREADY);

}

void recibir_pcb_dispatch(uint32_t *conexion){
	pcb* procesoRecibido = recibir_pcb(conexion);
	uint32_t cod_op = recibir_operacion(conexion);
	if(cod_op>0){
		switch(cod_op){
			case BLOCKED:
			pthread_mutex_lock(&COLABLOCK);
			queue_push(estadoBlock, procesoRecibido);
			printf("Proceso enviado a bloqueado");
			pthread_mutex_unlock(&COLABLOCK);
			case TERMINATED:
			pthread_mutex_lock(&COLAEXIT);
			queue_push(estadoExit, procesoRecibido);
			printf("Proceso enviado a EXIT");
			pthread_mutex_unlock(&COLAEXIT);
		}
	}
}

void calcularEstimacionPcb(pcb* proceso){
	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa); 
}



void planificadorSrt(uint32_t conexionDispatch, uint32_t conexionInterrupt){
	//FALTA: Sincronizar para enviar de a uno y recibir bien el estadoReady.
	
	pcb* primerElemento;
	pcb* segundoElemento;
	int i;

	uint32_t tamanioReady = queue_size(estadoReady);

	//Interrumpir lo que estan ejecutando en CPU.
	pthread_mutex_lock(&semInterrumpirCPU);
	interrumpirCPU = 1;
	send(conexionInterrupt, &interrumpirCPU, sizeof(uint32_t), NULL);
	pcb* elementoInterrumpido = recibir_pcb(conexionInterrupt);
	pthread_mutex_unlock(&semInterrumpirCPU);

	list_iterate(estadoReady, calcularEstimacionPcb);

	//Ordeno los elementos por su estimacion_actual.
	for(i=0; i<=tamanioReady; i++){
		primerElemento 	= list_get(estadoReady, i);
		segundoElemento = list_get(estadoReady, i+1);

		if(primerElemento->estimacion_rafaga_actual < segundoElemento->estimacion_rafaga_actual){
			list_replace(estadoReady, i, segundoElemento);
			list_replace(estadoReady, i + 1, primerElemento);
		} else {
			list_replace(estadoReady, i, primerElemento);
			list_replace(estadoReady, i+1, segundoElemento);
		}
	}

	//Envio los Procesos al CPU.
	//Mutex para enviar a dispatch.
	//pthread_mutex_lock(&semEnviarDispatch);
	pcb* elemEjecutar = queue_pop(estadoReady);
	paquete_pcb(elemEjecutar, conexionDispatch);
	printf ("Proceso enviado a CPU");
	
}

void planificadorFifo(uint32_t conexionDispatch){

	//Enviar Primer elemento de la lista a Cpu Dispatch
	pthread_mutex_lock(&semEnviarDispatch);
	pcb* elemEjecutar = queue_pop(estadoReady);
	paquete_pcb(elemEjecutar, conexionDispatch);
	printf("Proceso enviado a CPU");

}

//*****************************planificador a mediano plazo****************************



void *planificadorAMedianoPlazo(){

	u_int32_t conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);

	if(conexion_cpu_dispatch < 0){
		printf("Fallo a conectarse con cpu_dispatch");
		return EXIT_FAILURE;
	}

	uint32_t conexion_memoria = socket_connect_to_server(config_valores_memoria->ip, config_valores_memoria->puerto);

	if(conexion_memoria < 0){
		printf("Fallo al conectarse con memoria");
		return EXIT_FAILURE;
	}

	recibir_pcb_dispatch(conexion_cpu_dispatch);

	pthread_mutex_lock(&COLABLOCK);

	struct timeval initialBlock;
	struct timeval finalBlock;

	uint32_t tiempoMaxBlock = valores_generales->max_block;

	if(estadoBlock != NULL){

		pcb *procesoIO = queue_pop(estadoBlock);
		t_list *listaDeInstrucciones = procesoIO->instr;
		int *apunteProgCounter = procesoIO->programCounter;
		instr_t* instruccionBloqueada = list_get(listaDeInstrucciones, apunteProgCounter);
		uint32_t* tiempoIO = instruccionBloqueada->param;
	
		if(tiempoIO < tiempoMaxBlock){
			usleep(tiempoIO);
			queue_push(estadoReady, procesoIO);
			printf("Proceso bloqueado enviado devuelta a ready");
		}

		else if (tiempoIO > tiempoMaxBlock){

			gettimeofday(&initialBlock, NULL);
			usleep(tiempoMaxBlock);
			gettimeofday(&finalBlock, NULL);

			uint32_t tiempoRestanteBloqueo = finalBlock.tv_usec - initialBlock.tv_usec;
			queue_push(estadoBlockSusp, procesoIO);
			printf("Proceso bloqueado enviado a suspendido.");
			//TODO: Enviar mensaje a memoria.
			usleep(tiempoRestanteBloqueo);
			pcb *procesoASuspReady = queue_pop(estadoBlockSusp);
			queue_push(estadoReadySusp, procesoASuspReady);
			printf("Proceso enviado a suspended ready.");

		}	
	}	
	
		pthread_mutex_unlock(&COLABLOCK);
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
		if(gradoDeMultiProgActual < valores_generales->grad_multiprog && 
			queue_size (estadoNew)>0)
		 {	
			pthread_mutex_lock(&COLANEW);
			pcb *procesoAReady = queue_pop(estadoNew);
			pthread_mutex_unlock(&COLANEW); 

			uint32_t tablaDePaginas= obtenerTablaDePagina(procesoAReady);
			if(tablaDePaginas <0){
				perror("Error al asignar memoria al proceso");
				return EXIT_FAILURE;
			}
			procesoAReady->tablaDePaginas = tablaDePaginas;
			printf("\nProceso %d agregado con exito a la cola Ready",procesoAReady->id);
			printf("\nTabla de Pagina asignada: %d \n", procesoAReady->tablaDePaginas);

			queue_push(estadoReady, procesoAReady);
		 }
		pthread_mutex_unlock(&COLAREADY);
		pthread_mutex_unlock(&COLAEXEC);
		pthread_mutex_unlock(&COLABLOCK);

	 }
 }

 uint32_t obtenerTablaDePagina(pcb * pcb_proceso)
 {
	 uint32_t id;
	 uint32_t conexion= conectarse_con_memoria(); 
	t_paquete *paquete= crear_paquete(TABLADEPAGINA);
	agregarPcbAPaquete(paquete,pcb_proceso);
	enviar_paquete(paquete, conexion);

	recv(conexion, &id, sizeof(uint32_t), MSG_WAITALL); //se bloquea hasta recibir la respuesta
	eliminar_paquete(paquete);
	return id;
 }