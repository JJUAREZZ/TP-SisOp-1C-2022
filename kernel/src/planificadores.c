#include "../include/planificadores.h"
#include "../include/utils.h"
#include "../../shared/include/estructuras.h"
#include "sys/time.h"
#include <unistd.h>

//*****************************planificador a corto plazo****************************

void *planificadorACortoPlazo(){
	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

		if(utilizarFifo == 0){
				//Ejecutar FIFO.
				printf("\nPlanificador FIFO.\n");
				planificadorFifo();
		}

		if(utilizarSrt == 0){
				//Ejecuta SRT.
				printf ("\nPlanificador SRT.\n");
				planificadorSrt();
		}   
	
}


bool *menorEstimacion(pcb* proceso1, pcb* proceso2){
	uint32_t estimacion1 = proceso1->estimacion_rafaga_actual;
	uint32_t estimacion2 = proceso2->estimacion_rafaga_actual;
	return estimacion1 <= estimacion2;
}


void planificadorSrt(){
while(1){
	//espera si llega un proceso a ready ó si no hay procesos en running
	sem_wait(&semSrt);
 	pthread_mutex_lock(&COLAREADY);
 // si NO hay procesos en running y HAY procesos en ready
	if(queue_is_empty(estadoExec) && !queue_is_empty(estadoReady)){
		//obtiene el elemento con menor estimacion y lo envia
		list_sort(estadoReady->elements, menorEstimacion);
		pcb* proceso= queue_pop(estadoReady);
		pthread_mutex_lock(&COLAEXEC);
		queue_push(estadoExec,proceso);
		pthread_mutex_unlock(&COLAEXEC);
		paquete_pcb(proceso, socket_dispatch);
		printf("\nProceso %d enviado a CPU\n", proceso->id);
	}
	else if(queue_size(estadoReady) >0){
		//obtiene el elemento con menor estimacion
		list_sort(estadoReady->elements, menorEstimacion);
		pcb* proceso= queue_peek(estadoReady);
		//se interrumpe a la cpu para que mande el pcb y se queda a la espera
		uint32_t interrupt= DESALOJARPROCESO;
		interrupcion=1;
		send(socket_interrupt, &interrupt, sizeof(uint32_t), 0);
		printf("\nInterrupcion enviada a CPU\n");
		sem_wait(&semProcesoInterrumpido); //blocked y pcbdesalojado le dan signal 
		interrupcion=0;
		if(pcbDesalojado == NULL){
			paquete_pcb(proceso, socket_dispatch);
			printf("\nNo hay procesos en CPU");
			printf("\nProceso %d enviado a CPU\n", proceso->id);
			queue_pop(estadoReady);
			pthread_mutex_lock(&COLAEXEC);
			queue_push(estadoExec, proceso);
			pthread_mutex_unlock(&COLAEXEC);
		}
		//compara las rafagas
		//si la rafaga del desalojado es menor se devuelve a cpu
		else if((pcbDesalojado->estimacion_rafaga_actual - pcbDesalojado->cpu_anterior)
				<= proceso->estimacion_rafaga_actual){
			pthread_mutex_lock(&COLAEXEC);
			liberarPcb(queue_pop(estadoExec));//elimina el pcb viejo
			paquete_pcb(pcbDesalojado, socket_dispatch);
			printf("\nProceso desalojado %d devuelto a CPU\n", pcbDesalojado->id);
			queue_push(estadoExec, pcbDesalojado);
			pthread_mutex_unlock(&COLAEXEC);
			pcbDesalojado =NULL;
		}
		//si la rafaga del recien llegado es menor
		else if((pcbDesalojado->estimacion_rafaga_actual - pcbDesalojado->cpu_anterior)
				> proceso->estimacion_rafaga_actual){
			paquete_pcb(proceso, socket_dispatch);
			pthread_mutex_lock(&COLAEXEC);
			liberarPcb(queue_pop(estadoExec)); //elimina el pcb del desalojado
			queue_pop(estadoReady);
			queue_push(estadoExec,proceso);
			pthread_mutex_unlock(&COLAEXEC);
			queue_push(estadoReady,pcbDesalojado);
			printf("\nProceso %d enviado a CPU\n", proceso->id);
			pcbDesalojado= NULL;
		}
	}
	pthread_mutex_unlock(&COLAREADY);
}
 
}

void planificadorFifo(){
	while (1)
		{
			sem_wait(&semProcesosEnReady);
			sem_wait(&semProcesosEnRunning);
			if(queue_size(estadoReady)>0){
				pcb* elemEjecutar = queue_pop(estadoReady);
				queue_push(estadoExec,elemEjecutar);
				paquete_pcb(elemEjecutar, socket_dispatch);
				printf("Proceso %d enviado a CPU\n",elemEjecutar->id );
			}
			
		}
}

//*****************************planificador a mediano plazo****************************

void *planificadorAMedianoPlazo(){

 pthread_t hilo1;
 pthread_t  hilo2;
 pthread_create(&hilo1,NULL,enviarProcesosDeSuspendedReadyAReady,NULL);
 pthread_create(&hilo2,NULL,bloquearProcesos,NULL);

 pthread_detach(hilo1);
 pthread_detach(hilo2);

}


void *bloquearProcesos(){
	struct timeval initialBlock;
	struct timeval finalBlock;
	uint32_t tiempoMaxBlock = valores_generales->max_block;

	while(1){
		sem_wait(&semProcesosEnBlock);
		pcb *procesoIO = queue_peek(estadoBlock);
		t_list *listaDeInstrucciones = procesoIO->instr;
		int apunteProgCounter = procesoIO->programCounter;
		instr_t* instruccionBloqueada = list_get(listaDeInstrucciones, apunteProgCounter);
		uint32_t tiempoIO = instruccionBloqueada->param[0];
	
		if(tiempoIO < tiempoMaxBlock){
			usleep(tiempoIO*1000);
			procesoIO->programCounter += 1;
			queue_pop(estadoBlock);
			queue_push(estadoReady, procesoIO);
			printf("\nProceso %d bloqueado enviado devuelta a ready\n", procesoIO->id);
			sem_post(&semSrt);
			sem_post(&semProcesosEnReady);
			continue;
		}

		if(tiempoIO > tiempoMaxBlock){
			gettimeofday(&initialBlock, NULL);
			usleep(tiempoMaxBlock*1000);
			gettimeofday(&finalBlock, NULL);
			uint32_t tiempoBloqueo = time_diff_Mediano(&initialBlock, &finalBlock);
			uint32_t tiempoRestanteBloqueo = tiempoIO - tiempoBloqueo;
			queue_pop(estadoBlock);
			queue_push(estadoBlockSusp, procesoIO);
			printf("\nProceso %d bloqueado enviado a suspendido.\n", procesoIO->id);
			usleep(tiempoRestanteBloqueo*1000);
			//TODO: Enviar mensaje a memoria.

			pcb *procesoASuspReady = queue_pop(estadoBlockSusp);
			procesoIO->programCounter +=1;
			pthread_mutex_lock(&COLASUSPREADY);
			queue_push(estadoReadySusp, procesoASuspReady);
			pthread_mutex_unlock(&COLASUSPREADY);
			sem_post(&semProcesosEnSuspReady);
			printf("\nProceso %d enviado a suspended ready.\n",procesoIO->id);
			continue;
		}	
	}
}

void *enviarProcesosDeSuspendedReadyAReady(){
	while(1)
	 {
		sem_wait(&semProcesosEnSuspReady);
		pthread_mutex_lock(&COLAREADY);
		pthread_mutex_lock(&COLAEXEC);
		pthread_mutex_lock(&COLABLOCK);
		uint32_t gradoDeMultiProgActual= queue_size(estadoBlock)+
										 queue_size(estadoReady)+
										 queue_size(estadoExec);
		if(gradoDeMultiProgActual < valores_generales->grad_multiprog && 
			queue_size (estadoReadySusp)>0)
		 {	
			pthread_mutex_lock(&COLASUSPREADY);
			pcb *procesoAReady = queue_pop(estadoReadySusp);
			pthread_mutex_unlock(&COLASUSPREADY); 

			
			printf("\nProceso %d agregado con exito a la cola Ready",procesoAReady->id);
			printf("\nTabla de Pagina asignada: %d \n", procesoAReady->tablaDePaginas);
			queue_push(estadoReady, procesoAReady);
			sem_post(&semProcesosEnReady);
			sem_post(&semSrt);
		 }
		pthread_mutex_unlock(&COLAREADY);
		pthread_mutex_unlock(&COLAEXEC);
		pthread_mutex_unlock(&COLABLOCK);
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
	sem_post(&semProcesosEnNew);
	close(accepted_fd);
}

t_proceso* recibir_proceso(uint32_t accepted_fd){
	t_proceso* proceso;
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
	pcbDelProceso->estimacion_rafaga_anterior = 0;
	pcbDelProceso->cpu_anterior = 0;
	free(proceso);
	printf("\nPCB del proceso creado");
	return pcbDelProceso;
}


void planificadorALargoPlazo()
 {
	 pthread_t hilo1;
	 pthread_t  hilo2;
	 pthread_create(&hilo1,NULL,enviarProcesosAReady,NULL);
	 pthread_create(&hilo2,NULL,terminarProcesos,NULL);

	 pthread_detach(hilo1);
	 pthread_detach(hilo2);
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

 void enviarProcesosAReady()
 {
	 while(1)
	 {
		sem_wait(&semProcesosEnNew);
		pthread_mutex_lock(&COLAREADY);
		pthread_mutex_lock(&COLAEXEC);
		pthread_mutex_lock(&COLABLOCK);
		if(queue_is_empty(estadoReadySusp)){
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
				sem_post(&semProcesosEnReady);
				sem_post(&semSrt);
			}
		}
		
		pthread_mutex_unlock(&COLAREADY);
		pthread_mutex_unlock(&COLAEXEC);
		pthread_mutex_unlock(&COLABLOCK);
	 }
 }

 void terminarProcesos()
 {
	 while(1){
		 sem_wait(&semProcesosEnExit);
		 pthread_mutex_lock(&COLAEXIT);
		 liberarPcb(queue_pop(estadoExit));
		 pthread_mutex_unlock(&COLAEXIT);
		 if (interrupcion ==1)
			sem_post(&semProcesoInterrumpido);
		else 
			sem_post(&semSrt);
		 sem_post(&semProcesosEnRunning);
	     sem_post(&semProcesosEnNew);
		 sem_post(&semProcesosEnSuspReady);
	 }
}