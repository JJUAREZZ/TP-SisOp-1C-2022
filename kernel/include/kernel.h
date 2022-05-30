#ifndef SRC_KERNEL_H_
#define SRC_KERNEL_H_

#include "utils.h"
#include "planificadores.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

void *conectarse_con_consola();
void *recibir_proceso(int);
void *planificadorACortoPlazo();

t_log *logger;

void kernel_server_init(){

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	pthread_t conexion_con_consola;
	pthread_create(&conexion_con_consola, NULL, conectarse_con_consola, NULL); //HILO PRINCIPAL 
	pthread_join(conexion_con_consola, NULL);
	
}

//Ver Donde Colocarlo.
void* planificadorACortoPlazo(){
		
	t_list *argumentosPlanificadorCorto = list_create();
	list_add(argumentosPlanificadorCorto,estadoReady);
	list_add(argumentosPlanificadorCorto,logger);

	planificadorCorto(argumentosPlanificadorCorto);

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
	estadoNew 	= list_create();
	estadoReady = list_create();
	estadoBlock = list_create();
	estadoBlockSusp = list_create();
	estadoReadySusp = list_create();
	estadoExec = list_create();
	estadoExit = list_create();	
	for (;;) {
		int accepted_fd;
		if ((accepted_fd = accept(kernel_socket,(struct sockaddr *) &client_info, &addrlen)) != -1){

			pthread_t hilo;
			pthread_create(&hilo,NULL,recibir_proceso,accepted_fd);

			//FUNCIONA DESINCRONIZADO;
			//pthread_t planificador_corto;
			//pthread_create(&planificador_corto, NULL, planificadorACortoPlazo, NULL);


			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);

			
		}
	}
}

void *recibir_proceso(int accepted_fd){
	t_proceso* proceso= malloc(sizeof(t_proceso));
	pcb* pcbNuevo= malloc(sizeof(pcb)); 
	int cod_op;
	while (cod_op= recibir_operacion(accepted_fd)>0) {
		switch (cod_op) {
		case PAQUETE:
			proceso = recibir_paquete(accepted_fd);
			log_info(logger,"Me llego el siguiente proceso:\n");
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

			//Crear Hilo para crear PCB
		/*
			pthread_t hiloCreaPcb;  //por ahora quitamos los hilos, pero la idea es que haya 1 por
			pthread_t hiloPcbANew;	//cada planificador
			pthread_t hiloEnviarAReady;
		*/

			t_list *argumentosCrearPcb= list_create();
			list_add(argumentosCrearPcb,proceso);
			list_add(argumentosCrearPcb,pcbNuevo);
			list_add(argumentosCrearPcb,logger);

			t_list *argumentosEnviarAReady= list_create();
			list_add(argumentosEnviarAReady,estadoNew);
			list_add(argumentosEnviarAReady,logger);
			
		//Creamos listas con los parametros de las funciones para cuando usemos hilos. Ya que 
		//pthread_create sólo recibe la funcion y 1 parametro.
			crearPcb(argumentosCrearPcb);
			enviarAReady(argumentosEnviarAReady);



			return;
		case -1:
			log_info(logger,"el cliente se desconecto. Terminando servidor");
			return;
		default:
			log_info(logger,"Operacion desconocida. No quieras meter la pata");
			return;
		}
	}
}



#endif /* SRC_KERNEL_H_ */