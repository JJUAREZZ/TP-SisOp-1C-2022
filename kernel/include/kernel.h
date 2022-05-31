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
#include <commons/collections/queue.h>

void *conectarse_con_consola();
t_proceso *recibir_proceso(uint32_t);
void *atenderProceso(uint32_t);
void *planificadorACortoPlazo();
void *planificadorALargoPlazo();

t_log *logger;

void kernel_server_init(){

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	estadoNew 	= queue_create();
	estadoReady = queue_create();
	estadoBlock = list_create();
	estadoBlockSusp = list_create();
	estadoReadySusp = list_create();
	estadoExec = list_create();
	estadoExit = list_create();	
	gradoDeMultiprogramacionActual= 0;
	pthread_t conexion_con_consola;
	pthread_t planiALargoPlazo;
	pthread_t planiACortoPlazo;
	pthread_create(&conexion_con_consola, NULL, conectarse_con_consola, NULL); //HILO PRINCIPAL 
	pthread_create(&planiALargoPlazo, NULL, planificadorALargoPlazo, NULL); //HILO PLANI LARGO
	pthread_create(&planiACortoPlazo, NULL,planificadorACortoPlazo(), NULL); //HILO PLANI CORTO
	pthread_join(conexion_con_consola, NULL);
}



pthread_mutex_t mutexPlan_corto;
pthread_mutex_init(mutexPlan_corto); 

//Ver si utilizar
void* planificadorACortoPlazo(){

	pthread_mutex_lock(&mutexPlan_corto);
	planificadorCorto(estadoReady, logger);
	pthread_mutex_unlock(&mutexPlan_corto);

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
			pthread_t atenderProcesoNuevo;
			pthread_create(&atenderProcesoNuevo,NULL,atenderProceso,accepted_fd);
			log_info(logger,"Creando un hilo para atender una conexión en el socket %d", accepted_fd);
			
		}
	}
}

void *atenderProceso(uint32_t accepted_fd)
{
	t_proceso *nuevoProceso= recibir_proceso(accepted_fd);
	pcb* nuevoPcb= crearPcb(nuevoProceso);
	//mutex
	queue_push(estadoNew,nuevoPcb);
	//fin de mutex
}


t_proceso* recibir_proceso(uint32_t accepted_fd){
	t_proceso* proceso= malloc(sizeof(t_proceso));
	uint32_t cod_op= recibir_operacion(accepted_fd);
	if(cod_op>0){
		switch (cod_op)
		{
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
	//mutex para nro_proceso
	pcbDelProceso->id = nro_proceso;
	nro_proceso ++;
	//fin mutex
	pcbDelProceso->tamanioProceso= proceso->tamanio;
	pcbDelProceso->instr= proceso->instrucciones;
	pcbProceso->programCounter = 0;
	pcbDelProceso->tablaDePaginas = 0;
	pcbDelProceso->estimacion_rafaga_actual = valores_generales->est_inicial;
	free(proceso);
	printf("\nPCB del proceso creado");
	return pcbDelProceso;
}


void planificadorALargoPlazo()
 {
	 for(;;)
	 {

		 //falta checkear pcbs a finalizar

		 //mutex de las colas: ready, exec y blocked
		 uint32_t gradoDeMultiProgActual= queue_size(estadoBlock)+ queue_size(estadoReady)+
		 queue_size(estadoExec);
		 if(gradoDeMultiProgActual < valores_generales->grad_multiprog)
		 {
			 //enviar mensaje a memoria para obtener el valor de tabla de pagina
			 //wait memoria
			 //pcb *procesoAReady= queue_pop(estadoNew);
			 //procesoAReady->tablaDePaginas= lo que mande memoria
			 queue_push(estadoReady,queue_pop(estadoNew));
		 }
		 //fin del mutex
	 }
 }


#endif /* SRC_KERNEL_H_ */