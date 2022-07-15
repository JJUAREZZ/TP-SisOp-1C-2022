#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include "utils.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

t_log *logger;
pcb *unPcb;

sem_t semCicloInstruccion;



void *atenderPcb(uint32_t accepted_fd);
void atenderInterrupcion(uint32_t accepted_fd);
void ciclo_de_instruccion(uint32_t accepted_fd);
void fetch(pcb* pcb);
void devolverPcb(uint32_t co_op, uint32_t accepted_fd);

void *conectar_dispatcher()
{	
	sem_init(&semCicloInstruccion, 0, 0);
	
	logger = log_create("log.log", "Servidor Dispatcher", 1, LOG_LEVEL_DEBUG);

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	printf("Creando socket y escuchando \n");

	int cpu_dispatcher_socket = socket_create_listener(cpu_config->ip_cpu, cpu_config->puerto_escucha_dispatch);
	int cpu_interrupt_socket = socket_create_listener(cpu_config->ip_cpu, cpu_config->puerto_escucha_interrupt);

	if(cpu_dispatcher_socket < 0){
		log_info(logger, "Error al crear server dispatcher");
		return;
	}
	if(cpu_interrupt_socket < 0){
		log_info(logger, "Error al crear server interrupt");
		return;
	}
	log_info(logger, "¡¡¡Servidor dispatcher creado!!! Esperando Conexiones ...\n");
	log_info(logger, "¡¡¡Servidor interrupt creado!!! Esperando Conexiones ...\n");

	int accepted_fd_dispatch;
	int accepted_fd_interrupt;

	accepted_fd_dispatch = accept(cpu_dispatcher_socket,(struct sockaddr *) &client_info, &addrlen);
	socket_dispatch= accepted_fd_dispatch;
	pthread_t atenderNuevoPcb;
	pthread_create(&atenderNuevoPcb,NULL,atenderPcb,accepted_fd_dispatch);
	log_info(logger,"Creando un hilo para atender una conexion en el socket %d", accepted_fd_interrupt);

	accepted_fd_interrupt = accept(cpu_interrupt_socket,(struct sockaddr *) &client_info, &addrlen);
	socket_interrupt= accepted_fd_interrupt;
	pthread_t atenderNuevaInterrupcion;
	pthread_create(&atenderNuevaInterrupcion,NULL,atenderInterrupcion,accepted_fd_interrupt);
	log_info(logger,"Creando un hilo para atender una interrupcion en el socket %d", accepted_fd_interrupt);
		
	

	pthread_join(atenderNuevoPcb,NULL);
	pthread_join(atenderNuevaInterrupcion,NULL);

/*	for (;;) {
		int accepted_fd_dispatch;
		int accepted_fd_interrupt;
		if ((accepted_fd_dispatch = accept(cpu_dispatcher_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			socket_dispatch= accepted_fd_dispatch;
			pthread_t atenderNuevoPcb;
			pthread_create(&atenderNuevoPcb,NULL,atenderPcb,accepted_fd_dispatch);
			pthread_detach(atenderNuevoPcb);

			log_info(logger,"Creando un hilo para atender una conexion en el socket %d", accepted_fd_dispatch);
			
		}

		else
			log_info(logger,"conexion fallida");
		if ((accepted_fd_interrupt = accept(cpu_interrupt_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			socket_interrupt= accepted_fd_interrupt;
			pthread_t atenderNuevaInterrupcion;
			pthread_create(&atenderNuevaInterrupcion,NULL,atenderInterrupcion,accepted_fd_interrupt);
			printf("se conecto");
			log_info(logger,"Creando un hilo para atender una interrupcion en el socket %d", accepted_fd_interrupt);
		}

	}
*/
}

void *conectarse_con_memoria()
{
	memoria_config = malloc(sizeof(valores_config_memoria));
	uint32_t conexion= socket_connect_to_server(cpu_config->ip_memoria, cpu_config->puerto_memoria);
	if(conexion<0){
		log_info(logger, "Error al conectarse con Memoria");
		return EXIT_FAILURE;
	}
	socket_memoria= conexion;
	uint32_t cod_op= HANDSHAKE_CPU;
	send(socket_memoria, &cod_op, sizeof(uint32_t), 0);
	recv(socket_memoria, &cod_op, sizeof(uint32_t), MSG_WAITALL);
	if(cod_op>0)
	{
		switch (cod_op)
		{
			case HANDSHAKE_CPU:
			recv(socket_memoria, &memoria_config->tam_pagina, sizeof(uint32_t), MSG_WAITALL);
			recv(socket_memoria, &memoria_config->entradas_por_tabla, sizeof(uint32_t), MSG_WAITALL);
			printf("Valores de config de Memoria recibidos con exito");
			//log_info(logger, "Valores de config de Memoria recibidos con exito");
			return;
			break;
			
			default:
				;
		}
	}	
}



#endif /* SRC_CPU_H_ */