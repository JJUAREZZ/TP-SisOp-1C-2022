#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include "utils.h"
#include "planificadores.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>

t_log *logger;
void* planificadorACortoPlazo();
void *conectarse_con_consola();
uint32_t conectarse_con_memoria();
void conectarse_con_cpu();
void recalcularEstimacion(pcb* );

void kernel_server_init(){
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	sem_init(&semProcesosEnReady,0,0);
	sem_init(&semProcesosEnRunning,0,1);
	sem_init(&semProcesosEnBlock,0,0);
	sem_init(&semProcesosEnExit,0,0);
	sem_init(&semProcesosEnNew,0,0);
	sem_init(&semProcesoInterrumpido,0,0);
	//sem_init(&semProcesosOrdenados, 0, 0);
	sem_init(&semSrt, 0, 0);
	estadoNew 	= queue_create();
	estadoReady = queue_create();
	estadoBlock = queue_create();
	estadoBlockSusp = queue_create();
	estadoReadySusp = queue_create();
	estadoExec = queue_create();
	estadoExit = queue_create();
	pthread_mutex_init(&COLANEW, NULL);
	pthread_mutex_init(&COLAREADY, NULL);
	pthread_mutex_init(&COLAEXEC, NULL);
	pthread_mutex_init(&COLABLOCK, NULL);
	pthread_mutex_init(&COLABLOCKREADY, NULL);
	pthread_mutex_init(&COLABLOCKSUSP, NULL);
	pthread_mutex_init(&COLAEXIT, NULL);
	pthread_mutex_init(&PROCDESALOJADO, NULL);
	pthread_mutex_init(&semEnviarDispatch, NULL);
	//pthread_mutex_init(&semInterrumpirCPU, NULL);
	pcbDesalojado=NULL;

	pthread_t conexion_con_consola;
	pthread_t conexion_con_cpu;
	pthread_t planiALargoPlazo;
	pthread_t planiAMedianoPlazo;
	pthread_t planiACortoPlazo;
	pthread_create(&conexion_con_consola, NULL, conectarse_con_consola, NULL);
	pthread_create(&conexion_con_cpu, NULL, conectarse_con_cpu, NULL);
	pthread_create(&planiALargoPlazo, NULL, planificadorALargoPlazo, NULL); //HILO PLANI LARGO
	pthread_create(&planiACortoPlazo, NULL,planificadorACortoPlazo, NULL); //HILO PLANI CORTO
	//pthread_create(&planiAMedianoPlazo, NULL, planificadorAMedianoPlazo, NULL); //HILO PLANI MEDIANO.

	pthread_join(conexion_con_consola, NULL);
	pthread_join(planiALargoPlazo, NULL);
	pthread_join(planiACortoPlazo, NULL);
	//pthread_join(planiAMedianoPlazo, NULL);
}

void *conectarse_con_consola()
{
	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	printf("Creando socket y escuchando \n");

	kernel_socket = socket_create_listener(config_valores_kernel->ip, config_valores_kernel->puerto);

	if(kernel_socket < 0){
		log_info(logger, "Error al crear server");
		return;
	}
	log_info(logger, "¡¡¡Servidor de Kernel Creado!!! Esperando Conexiones ...\n");
	for (;;) {
		int accepted_fd;
		if ((accepted_fd = accept(kernel_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			printf("\n");
			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);
			pthread_t atenderProcesoNuevo;
			pthread_create(&atenderProcesoNuevo,NULL,atenderProceso,accepted_fd);
		}
	}
}

void conectarse_con_cpu()
{ 
	uint32_t socket1= socket_connect_to_server(config_valores_cpu_dispatch->ip,
	 											config_valores_cpu_dispatch->puerto);
	uint32_t socket2= socket_connect_to_server(config_valores_cpu_interrupt->ip,
	 											config_valores_cpu_interrupt->puerto);
	
	if(socket1<0)
		return EXIT_FAILURE;
	else
		socket_dispatch= socket1;

	if(socket2<0)
			return EXIT_FAILURE;
	else
		socket_interrupt= socket2;

	while(1)
	{
		uint32_t cod_op= recibir_operacion(socket_dispatch);
		if(cod_op>0)
		{
			switch (cod_op)
			{
				pcb* procesoAExit;
				pcb* procesoABlocked;

				case PROCESOTERMINATED:
					procesoAExit= recibir_pcb(socket_dispatch);
					//agregar mutex
					pthread_mutex_lock(&COLAEXIT);
					queue_push(estadoExit,procesoAExit);
					pthread_mutex_unlock(&COLAEXIT);
					sem_post(&semProcesosEnExit);
					break;
				case BLOCKED : 
					procesoABlocked = recibir_pcb(socket_dispatch);
					recalcularEstimacion(procesoABlocked);
					//agregar mutex
					queue_push(estadoBlock, procesoABlocked);
					sem_post(&semProcesosEnBlock);
					sem_post(&semProcesosEnRunning);
					sem_post(&semProcesoInterrumpido);
					break;
				case PROCESODESALOJADO : 
					//se almacena en la var global el pcb desalojado
					pthread_mutex_lock(&PROCDESALOJADO);
					pcbDesalojado = recibir_pcb(socket_dispatch);
					pthread_mutex_unlock(&PROCDESALOJADO);
					sem_post(&semProcesoInterrumpido);
				default:
					;
			}
		}
	}	
}


void recalcularEstimacion(pcb* proceso){
	double alfa= valores_generales->alfa;
	uint32_t realAnterior= proceso->cpu_anterior ;
	uint32_t estimadoAnterior= proceso->estimacion_rafaga_anterior;
	proceso->estimacion_rafaga_actual= (realAnterior* alfa) + (estimadoAnterior * (1-alfa));
}

uint32_t conectarse_con_memoria()
{
	uint32_t conexion= socket_connect_to_server(config_valores_memoria->ip, config_valores_memoria->puerto);
	if(conexion<0)
		return EXIT_FAILURE;
	return conexion;
}



#endif /* SRC_KERNEL_H_ */
