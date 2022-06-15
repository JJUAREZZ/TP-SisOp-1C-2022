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

void *atenderPcb(uint32_t accepted_fd);
void atenderInterrupcion(uint32_t accepted_fd);
void ciclo_de_instruccion(pcb* pcb);
void fetch(pcb* pcb);
void devolverPcb(pcb* unPcb, uint32_t co_op, uint32_t accepted_fd);

void *conectar_dispatcher()
{
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

	for (;;) {
		int accepted_fd_dispatch;
		int accepted_fd_interrupt;
		if ((accepted_fd_dispatch = accept(cpu_dispatcher_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){

			pthread_t atenderNuevoPcb;
			pthread_create(&atenderNuevoPcb,NULL,atenderPcb,accepted_fd_dispatch);
			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd_dispatch);

			
		}
		/*if ((accepted_fd_interrupt = accept(cpu_interrupt_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			pthread_t atenderNuevaInterrupcion;
			pthread_create(&atenderNuevaInterrupcion,NULL,atenderInterrupcion,accepted_fd_interrupt);
			log_info(logger,"Creando un hilo para atender una interrupcion en el socket %d", accepted_fd_interrupt);
		}*/
	}
}

#endif /* SRC_CPU_H_ */