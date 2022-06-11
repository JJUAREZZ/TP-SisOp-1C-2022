#include "../include/planificadores.h"
#include "../../shared/include/estructuras.h"

//*****************************planificador a corto plazo****************************

void *planificadorACortoPlazo(){
	pthread_mutex_lock(&COLABLOCKREADY);

	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

	if(utilizarFifo == 0){
			//Ejecutar FIFO.
			printf("Planificador por FIFO.");
			planificadorFifo();
	}

	if(utilizarSrt == 0){
			//hilo que ejecuta SRT.
			printf ("Planificador por SRT.");
			planificadorSrt();
	}   
	pthread_mutex_unlock(&COLABLOCKREADY);
}

void calcularEstimacionPcb(pcb* proceso){
	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa); 
}

void planificadorSrt(){
	//FALTA: Sincronizar para enviar de a uno y recibir bien el estadoReady.
	//FALTA: Hacer que desaloje el proceos de CPU.
	
	u_int32_t tamanioReady = queue_size(estadoReady);
	u_int32_t conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);
	u_int32_t conexion_cpu_interrupt = socket_connect_to_server(config_valores_cpu_interrupt->ip, config_valores_cpu_interrupt->puerto);

	while(tamanioReady > 0){

	//Calculo las estimaciones de cada PCB
	list_iterate(estadoReady, calcularEstimacionPcb);

	pcb* primerElemento;
	pcb* segundoElemento;
	int i;

	//Interrumpir lo que estan ejecutando en CPU.

	pthread_mutex_lock(&semInterrumpirCPU);
	interrumpirCPU = 1;
	send(conexion_cpu_interrupt, &interrumpirCPU, sizeof(uint32_t), NULL);
	//FALTA: HACER FUNCION QUE RECIBA PROCESO DE CPU INTERRUPT.
	pthread_mutex_unlock(&semInterrumpirCPU);

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
	pthread_mutex_lock(&semEnviarDispatch);
	pcb* elemEjecutar = queue_pop(estadoReady);
	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	printf ("Proceso enviado a CPU");

	}	
}

/*recibir_pcb_interrupt(){
	recv()
}
*/

void planificadorFifo(){

/*
	u_int32_t tamanioReady = queue_size(estadoReady);
	u_int32_t conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);

	while(tamanioReady > 0){ //quizas un semaforo de tipo productor consumidor. No espera activa, debería bloquearse

	//Enviar Primer elemento de la lista a Cpu Dispatch
	pthread_mutex_lock(&semEnviarDispatch);
	pcb* elemEjecutar = queue_pop(estadoReady);
	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	printf("Proceso enviado a CPU");
	}
*/

	while (1)
		{
			sem_wait(semProcesosEnReady);
			sem_wait(semProcesosEnRunning);

			

		}

	}

//*****************************planificador a mediano plazo****************************

void *planificadorAMedianoPlazo(){

	//Enviar de bloqueado a bloqueado suspendido.

	uint32_t tamanioBlocked = queue_size(estadoBlock);
	int i;
	pcb* procesoBloqueado;
	
	for(i=0; i<=tamanioBlocked; i++){
		procesoBloqueado = list_get(estadoBlock, i);

		//Supuestamente el programCounterDeberia estar apuntando a la instruccion IO.

		t_list* listaDeInstrucciones = procesoBloqueado->instr;
		int* apunteProgCounter = procesoBloqueado->programCounter;
		instr_t* instruccionBloqueada = list_get(listaDeInstrucciones, apunteProgCounter);
		uint32_t* tiempoIO = instruccionBloqueada->param;

		//Si el tiempo en IO es mayor al limite de bloqueo enviar a block suspended.
		if(tiempoIO > valores_generales->max_block){
			list_remove(estadoBlock, i);
			queue_push(estadoBlockSusp, procesoBloqueado);
			//FALTA: Enviar mensaje a memoria.
		}
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
	close(accepted_fd);
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
	 pthread_t hilo1;
	 pthread_t  hilo2;
	 pthread_create(&hilo1,NULL,enviarProcesosAReady,NULL);
	 pthread_create(&hilo2,NULL,terminarProcesos,NULL);

	 pthread_join(hilo1, NULL);
	 pthread_join(hilo2, NULL);
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
 void terminarProcesos()
 {
	 while(1){
		 ;
	 }
 }