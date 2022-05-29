#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/config.h>
#include <pthread.h>


int kernel_socket;

//Estructura para poder escuchar y conectarnos a todos los puertos
typedef struct {
	char* ip;
	char* puerto;
} config_conex;

//Estructura para poder realizar PCB.
typedef struct {
	char* alg_planif;
	int est_inicial;
	double alfa;
	int grad_multiprog;
	int max_block;
} gralStruct;


t_list* estadoNew;
t_list* estadoReady;
t_list* estadoBlock;
t_list* estadoBlockSusp;
t_list* estadoReadySusp;
t_list* estadoExec;
t_list* estadoExit;

config_conex* config_valores_kernel;
config_conex* config_valores_memoria;
config_conex* config_valores_cpu_dispatch;
config_conex* config_valores_cpu_interrupt;
//falta puerto escucha.

gralStruct *valores_generales;


void load_configuration(){

	t_config* config = config_create("/home/utnso/workspace/tp-2022-1c-Messirve/kernel/cfg/kernel.config");


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.


	config_valores_kernel = malloc(sizeof(config_conex));
	config_valores_kernel->ip = config_get_string_value(config, "IP_KERNEL");
	config_valores_kernel->puerto = config_get_int_value(config, "PUERTO_KERNEL");

	config_valores_memoria = malloc(sizeof(config_conex*));
	config_valores_memoria->ip = config_get_string_value(config, "IP_MEMORIA");
	config_valores_memoria->puerto = config_get_int_value(config, "PUERTO_MEMORIA");

	config_valores_cpu_dispatch = malloc(sizeof(config_conex*));
	config_valores_cpu_dispatch->ip =config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	config_valores_cpu_dispatch->puerto = config_get_int_value(config, "PUERTO_CPU_DISPATCH");

	config_valores_cpu_interrupt = malloc(sizeof(config_conex*));
	config_valores_cpu_interrupt->ip = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	config_valores_cpu_interrupt->puerto = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");

	//Lleno los struct de los campos que necesitamos para el pcb y demas.
	valores_generales = malloc(sizeof(gralStruct));
	valores_generales->alg_planif = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	valores_generales->est_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	valores_generales->alfa = config_get_double_value(config, "ALFA");
	valores_generales->grad_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	valores_generales->max_block = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");

	//kernel_logger_info("IP_KERNEL: %s", config_valores->ip_kernel);
	//kernel_logger_info("PUERTO_KERNEL: %d", config_valores->puerto_kernel);
}

int nro_proceso = 0;

//MUTEX PARA QUE UNO A LA VEZ CREEN EL PCB

pthread_mutex_t mutexPcb;
pthread_mutex_init(mutexPcb);

//void* crearPcb(t_proceso* procesoA, pcb* pcbProceso_a, t_log* unLogger){
	void* crearPcb (t_list *arg){
	t_proceso* procesoA= list_get(arg,0);
	pcb* pcbProceso_a= list_get(arg,1);
	t_log* unLogger= list_get(arg,2);

	pthread_mutex_lock(&mutexPcb);
	pcbProceso_a->id = nro_proceso;
	pcbProceso_a->tamanioProceso = procesoA->tamanio;
	pcbProceso_a->instr = procesoA->instrucciones;
	pcbProceso_a->programCounter = 0;
	pcbProceso_a->tablaDePaginas = 0;
	pcbProceso_a->estimacion_rafaga = valores_generales->est_inicial;
	nro_proceso++ ;
	log_info(unLogger, "PCB del proceso arrivado creado");
	list_add(estadoNew,pcbProceso_a);
	pthread_mutex_unlock(&mutexPcb);
	
}

void *planificadorCorto(t_list* arg){

	t_list* listaNew = list_get(arg,0);
	t_log* unLogger  = list_get(arg,1);	

	int	tamanioReady = list_size(estadoReady);
	int gradoMultiprogramacion = valores_generales->grad_multiprog;

	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

	if(tamanioReady <= gradoMultiprogramacion){
		pcb* elemEnviar = list_remove(listaNew, 0);
		if(utilizarFifo == 0){
			//Ejecutar FIFO.
			//Esta linea no va a ser falta
			log_info(unLogger, "Planificador por FIFO.");
			planificadorFifo(elemEnviar, unLogger);
		}	
	if(utilizarSrt == 0){
			//hilo que ejecuta SRT.
			log_info(unLogger, "Planificador por SRT.");
			planificadorSrt(elemEnviar, unLogger);
		}
	} else {
		log_info(unLogger, "El grado de multiprogramacion no permite mas elementos");
	}
}


void planificadorSrt(t_list listaReadt, t_log unLogger){
/*	int tamanioReady = list_size(listaReady);
	while(tamanioReady > 0){
		
	}*/
}

//ver si tiene que agarrar un proceso o una lista de ready.
void planificadorFifo(pcb* unProceso, t_log* unLogger){

	int tamanioReady = list_size(estadoReady);
	list_add(estadoReady, unProceso);
	log_info(unLogger, "Proceso arriba en ready");

	while(tamanioReady > 0){
	pcb* elemEjecutar = list_remove(estadoReady, 0);
	//Enviar Primer elemento de la lista a Cpu Dispatch
	int conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);
	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	log_info(unLogger, "Proceso enviado a CPU");

	//Recibir de blocked los Procesos.
	pcb* elemRecib; 

	}
}



#endif /* SRC_UTILS_H_ */