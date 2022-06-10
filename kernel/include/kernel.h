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

void kernel_server_init(){
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
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
	pthread_mutex_init(&semEnviarDispatch, NULL);
	//pthread_mutex_init(&semInterrumpirCPU, NULL);

	pthread_t conexion_con_consola;
	pthread_t planiALargoPlazo;
	pthread_t planiAMedianoPlazo;
	pthread_t planiACortoPlazo;
	pthread_create(&conexion_con_consola, NULL, conectarse_con_consola, NULL); //HILO PRINCIPAL 
	pthread_create(&planiALargoPlazo, NULL, planificadorALargoPlazo, NULL); //HILO PLANI LARGO
	//pthread_create(&planiACortoPlazo, NULL,planificadorACortoPlazo, NULL); //HILO PLANI CORTO
	//pthread_create(&planiAMedianoPlazo, NULL, planificadorAMedianoPlazo, NULL); //HILO PLANI MEDIANO.

	//pthread_join(conexion_con_consola, NULL);
	pthread_join(planiALargoPlazo, NULL);
	pthread_join(planiACortoPlazo, NULL);
	pthread_join(planiAMedianoPlazo, NULL);
	pthread_join(conexion_con_consola, NULL); 
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
			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);
			pthread_t atenderProcesoNuevo;
			pthread_create(&atenderProcesoNuevo,NULL,atenderProceso,accepted_fd);
			
		}
	}
}

uint32_t conectarse_con_memoria()
{
	uint32_t conexion= socket_connect_to_server(config_valores_memoria->ip, config_valores_memoria->puerto);
	if(conexion<0)
		return EXIT_FAILURE;
	return conexion;
}

#endif /* SRC_KERNEL_H_ */
