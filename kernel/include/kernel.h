#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include "utils.h"
#include <pthread.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

/*
int create_kernel_logger(){
	int response = kernel_logger_create("kernel_logger.log");
	if(response<0){
		free(config_valores->ip_kernel);
		free(config_valores->puerto_kernel);
		free(config_valores);
	
	}
	else{
		kernel_logger_info("Kernel Logger creado");
	}
}
*/

void kernel_server_init(){

	t_log* logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	printf("Creando socket y escuchando \n");

	kernel_socket = socket_create_listener(config_valores_kernel->ip, config_valores_kernel->puerto);

	if(kernel_socket < 0){
		//kernel_logger_error("Error al crear server");
		log_info(logger, "Error al crear server");
		return;
	}

	//kernel_logger_info("Kernel Server created !!! Waiting Connections ...\n");
	log_info(logger, "Kernel Server created !!! Waiting Connections ...\n");


	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;

	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	int accepted_fd;
		for (;;) {

		pthread_t tid;
		if ((accepted_fd = accept(kernel_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){
			//kernel_logger_info("Creando un hilo para atender una conexión en el socket %d", accepted_fd);
			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);
		}
		else{
			log_info(logger,"Error al conectar con un cliente");
		}
	t_proceso* proceso;
	pcb* pcbNuevo; 

	estadoNew 	= list_create();
	estadoReady = list_create();
	estadoBlock = list_create();
	estadoBlockSusp = list_create();
	estadoReadySusp = list_create();
	estadoExec = list_create();
	estadoExit = list_create();	

	while (1) {
		int cod_op = recibir_operacion(accepted_fd);
		switch (cod_op) {
		case PAQUETE:
			proceso = recibir_paquete(accepted_fd);
			log_info(logger,"Me llego el siguiente proceso:\n");
			printf("Tamanio del Proceso en bytes: %d", proceso->tamanio);
			printf("\nInstrucciones : \n");
			void mostrarInstrucciones(instr_t* element)
			{
				printf("%s ",element->id);
				for(int i=0; i<element->nroDeParam;i++){
					printf(" %d",(int) element->param[i]);
				}
				printf("\n");
			}
			list_iterate(proceso->instrucciones, mostrarInstrucciones);

			//Crear Hilo para crear PCB
			pthread_t hiloCreaPcb;
			pthread_create(hiloCreaPcb, NULL, crearPcb(proceso, pcbNuevo, logger), NULL);

			pthread_t hiloPcbANew;
			pthread_create(hiloPcbANew, NULL, list_add(estadoNew, pcbNuevo), NULL);

			//Extrae elemento de New y envia a READY
			//****FALTA MODIFICAR EL CAMPO TABLA DE PAGINAS****
			//thread_t hiloEnviarAReady;
			//pthread_create(hiloEnviarAReady, NULL, enviarAReady(estadoNew, logger), NULL);

			//QUE SE EJECUTE UN HILO A LA VEZ
			pthread_join(hiloCreaPcb, NULL);
			pthread_join(hiloPcbANew, NULL);
			//pthread_join(hiloEnviarAReady, NULL);

			break;
		case -1:
			log_info(logger,"el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_info(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;


	}
	
}



#endif /* SRC_KERNEL_H_ */