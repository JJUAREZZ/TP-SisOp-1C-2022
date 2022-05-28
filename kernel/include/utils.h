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

void* crearPcb(t_proceso* procesoA, pcb* pcbProceso_a, t_log* unLogger){
	
	pthread_mutex_lock(&mutexPcb);
	pcbProceso_a->id = nro_proceso;
	pcbProceso_a->tamanioProceso = procesoA->tamanio;
	pcbProceso_a->instr = procesoA->instrucciones;
	pcbProceso_a->programCounter = 0;
	pcbProceso_a->tablaDePaginas = 0;
	pcbProceso_a->estimacion_rafaga = valores_generales->est_inicial;
	nro_proceso++ ;
	log_info(unLogger, "PCB del proceso arrivado creado");
	pthread_mutex_unlock(&mutexPcb);
}

//En primer lugar va a enviar de estado new a ready siempre que la multiprogramacion lo permita
void *enviarAReady(t_list* estado, t_log* unLogger){

    int	tamanioReady = list_size(estadoReady);
	int gradoMutlriprogramacion = valores_generales->grad_multiprog;

	if(tamanioReady <= gradoMutlriprogramacion){ 
		//SACA ELEMENTO DE NEW 
		pcb* elemEnviar = list_get(estado, 1);		

		//ENVIAR Y RECIBIR MENSAJE A MEMORIA PARA MODIFICAR LA TLB DEL PCB EXTRAIDO

		//AGREGA ELEMENTO A READY
		list_add(estadoReady, elemEnviar);
		log_info(unLogger, "Proceso enviado a ready");
	}else{
		log_info(unLogger, "El grado de multiprogramacion no permite enviar otro proceso a READY.");
	}

}

#endif /* SRC_UTILS_H_ */