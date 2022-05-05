#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include "utils.h"
#include "../shared/sockets.h"

int create_kernel_logger(){
	int response = kernel_logger_create("kernel_logger.log");

	if(response<0){
		/*free(config_valores->ip_kernel);
		free(config_valores->puerto_kernel);
		free(config_valores);*/
	}
	else{
		kernel_logger_info("Kernel Logger creado");
	}

}

void kernel_server_init(){

	printf("Creando socket y escuchando \n");

	kernel_socket = socket_create_listener(config_valores->ip_kernel, config_valores->puerto_kernel);

	if(kernel_socket < 0){
		kernel_logger_error("Error al crear server");
		return;
	}

	kernel_logger_info("Kernel Server created !!! Waiting Connections ...\n");

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;

	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	int accepted_fd;
		for (;;) {

		pthread_t tid;
		if ((accepted_fd = accept(kernel_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			kernel_logger_info("Creando un hilo para atender una conexión en el socket %d", accepted_fd);
		}
		else{
			kernel_logger_error("Error al conectar con un cliente");
		}
	}
}

#endif /* SRC_KERNEL_H_ */
