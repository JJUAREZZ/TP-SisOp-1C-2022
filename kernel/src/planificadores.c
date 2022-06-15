#include "../include/planificadores.h"
#include "../../shared/include/estructuras.h"
#include "sys/time.h"
#include <unistd.h>

//*****************************planificador a corto plazo****************************

void *planificadorACortoPlazo(){
	//pthread_mutex_lock(&COLABLOCKREADY);

	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

		if(utilizarFifo == 0){
				//Ejecutar FIFO.
				printf("\nPlanificador  FIFO.\n");
				planificadorFifo();
		}

		if(utilizarSrt == 0){
				//Ejecuta SRT.
				printf ("\nPlanificador por SRT.\n");
				planificadorSrt();
		}   
	
}

void calcularEstimacionPcb(pcb* proceso){
	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior  * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa);
}

bool *menorEstimacion(pcb* proceso1, pcb* proceso2){
	uint32_t estimacion1 = proceso1->estimacion_rafaga_actual;
	uint32_t estimacion2 = proceso2->estimacion_rafaga_actual;
	return estimacion1 <= estimacion2;
}

void *enviarProcesosOrdenados(){

	//Envio los Procesos al CPU.
	while(1){

	sem_wait(&semProcesosOrdenados);
	sem_wait(&semProcesosEnRunning);
	pcb* elemEjecutar = queue_pop(estadoReady);
	paquete_pcb(elemEjecutar, socket_dispatch);
	printf("Proceso %d enviado a CPU\n", elemEjecutar->id );

	}

}	

void* ordenarProcesos(){

	pcb* procEnReady;
	pcb* elemMenEstimacion;
	pcb* primerElemento;
	int i;
	uint32_t flagInterrupcion;

	while(1){
	
	sem_wait(&semProcesosEnReady);

	//INTERRUMPIR LO EJECUTADO EN CPU
	

	uint32_t tamanioReady = queue_size(estadoReady);
	printf("El tamanio de ready es: %d \n", tamanioReady);

	for(i=0; i<tamanioReady; i++){
		primerElemento = list_get(estadoReady->elements, i);
		calcularEstimacionPcb(primerElemento);
	}
	
	list_sort(estadoReady->elements, menorEstimacion);
	elemMenEstimacion = list_get(estadoReady->elements, 0);
	printf("EL ELEMENTO %d TIENE LA MENOR ESTIMACION : %d\n", elemMenEstimacion->id, elemMenEstimacion->estimacion_rafaga_actual);
	
	sem_post(&semProcesosOrdenados);

	}

}

void planificadorSrt(){
	//pthread_t hilo1;
	//pthread_t hilo2;

while(1){
	//espera si llega un proceso a ready ó si no hay procesos en running
	sem_wait(&semSrt);
 
 // si NO hay procesos en running y HAY procesos en ready
	if(queue_size(estadoExec) ==0 && queue_size(estadoReady >0)){
		//obtiene el elemento con menor estimacion y lo envia
		pcb* proceso= list_get_minimun(estadoReady->elements, menorEstimacion);
		paquete_pcb(proceso, socket_dispatch);
		printf("\nProceso enviado a CPU\n");
	}
	else if(queue_size(estadoReady >0)){
		//obtiene el elemento con menor estimacion
		pcb* proceso= list_get_minimun(estadoReady->elements, menorEstimacion);
		//se interrumpe a la cpu para que mande el pcb y se queda a la espera
		send(socket_interrupt, DESALOJARPROCESO, sizeof(uint32_t), 0);
		sem_wait(&semProcesoInterrumpido);

		if(pcbDesalojado == NULL){
			paquete_pcb(proceso, socket_dispatch);
			printf("\nProceso enviado a CPU\n");
		}
		//compara las rafagas
		//si la rafaga del desalojado es menor se devuelve a cpu
		else if(pcbDesalojado->estimacion_rafaga_actual <= proceso->estimacion_rafaga_actual){
			paquete_pcb(pcbDesalojado, socket_dispatch);
			printf("\nProceso desalojado devuelto a CPU\n");
			liberarPcb(pcbDesalojado);
			pcbDesalojado =NULL;
		}
		//si la rafaga del recien llegado es menor
		else if(pcbDesalojado->estimacion_rafaga_actual > proceso->estimacion_rafaga_actual){
			bool condition(pcb* element){
				return element->id == proceso->id;
			}
			//se elimina de la cola ready ese pcb
			list_remove_by_condition(estadoReady->elements, condition);
			//se envia dicho pcb
			paquete_pcb(proceso, socket_dispatch);
			printf("\nNuevo proceso enviado a CPU\n");
			liberarPcb(proceso);
			pcb* pcbDesalojadoActualizado= pcbDesalojado;
			pcbDesalojado= NULL;
			//encolo el pcb desalojado a ready otra vez
			queue_push(estadoReady, pcbDesalojadoActualizado);
		}
	}
}


	//pthread_create(&hilo1,NULL,ordenarProcesos , NULL);
	//pthread_create(&hilo2,NULL,enviarProcesosOrdenados ,NULL);

	//pthread_join(&hilo1, NULL);
	//pthread_join(&hilo2, NULL);

}

void planificadorFifo(){
	while (1)
		{
			sem_wait(&semProcesosEnReady);
			sem_wait(&semProcesosEnRunning);
			pcb* elemEjecutar = queue_pop(estadoReady);
			paquete_pcb(elemEjecutar, socket_dispatch);
			printf("Proceso enviado a CPU\n");
		}
}

//*****************************planificador a mediano plazo****************************



void *planificadorAMedianoPlazo(){

	pthread_mutex_lock(&COLABLOCK);

	struct timeval initialBlock;
	struct timeval finalBlock;

	uint32_t tiempoMaxBlock = valores_generales->max_block;

	while(1){
	sem_wait(&semProcesosEnBlock);
	
		pcb *procesoIO = queue_pop(estadoBlock);
		t_list *listaDeInstrucciones = procesoIO->instr;
		int *apunteProgCounter = procesoIO->programCounter;
		instr_t* instruccionBloqueada = list_get(listaDeInstrucciones, apunteProgCounter);
		uint32_t* tiempoIO = instruccionBloqueada->param;
	
		if(tiempoIO < tiempoMaxBlock){
			usleep(tiempoIO);
			queue_push(estadoReady, procesoIO);
			printf("Proceso bloqueado enviado devuelta a ready");
			sem_post(&semProcesosEnReady);
		}

		else if (tiempoIO > tiempoMaxBlock){

			gettimeofday(&initialBlock, NULL);
			usleep(tiempoMaxBlock);
			gettimeofday(&finalBlock, NULL);

			uint32_t tiempoBloqueo = finalBlock.tv_usec - initialBlock.tv_usec;
			uint32_t tiempoRestanteBloqueo = tiempoIO - tiempoBloqueo;
			queue_push(estadoBlockSusp, procesoIO);
			printf("Proceso bloqueado enviado a suspendido.");
			usleep(tiempoRestanteBloqueo);
			//TODO: Enviar mensaje a memoria.

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
	pcbDelProceso->estimacion_rafaga_anterior = valores_generales->est_inicial;
	pcbDelProceso->cpu_anterior = 1;
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
		sem_post(&semProcesosEnReady);

	 }

 }
 void terminarProcesos()
 {
	 while(1){
		 sem_wait(&semProcesosEnExit);
		 pcb* procesoATerminar= queue_pop(estadoExit);
		 pcb* procesoEnEjecucion= queue_pop(estadoExec);
		 if(procesoATerminar->id != procesoEnEjecucion->id)
		 {
			 printf("\nError. Proceso a terminar no estaba en ejecucion");
			 return EXIT_FAILURE;
		 }
		 liberarPcb(procesoEnEjecucion);
	 }

 }