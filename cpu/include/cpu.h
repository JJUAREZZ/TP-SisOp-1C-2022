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

pcb* recibir_pcb(uint32_t accepted_fd);
void *atenderPcb(uint32_t accepted_fd);
void fetch(pcb* pcb);

void *conectar_dispatcher()
{
	logger = log_create("log.log", "Servidor Dispatcher", 1, LOG_LEVEL_DEBUG);

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	printf("Creando socket y escuchando \n");

	int cpu_dispatcher_socket = socket_create_listener(cpu_config.ip_dispatch, cpu_config.puerto_escucha_dispatch);

	if(cpu_dispatcher_socket < 0){
		log_info(logger, "Error al crear server");
		return;
	}

	log_info(logger, "¡¡¡Servidor dispatcher creado!!! Esperando Conexiones ...\n");
	for (;;) {
		int accepted_fd;
		if ((accepted_fd = accept(cpu_dispatcher_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){

			pthread_t atenderNuevoPcb;
			pthread_create(&atenderNuevoPcb,NULL,atenderPcb,accepted_fd);

			//FUNCIONA DESINCRONIZADO;
			//pthread_t planificador_corto;
			//pthread_create(&planificador_corto, NULL, planificadorACortoPlazo, NULL);


			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);

			
		}
	}
}

#endif /* SRC_CPU_H_ */