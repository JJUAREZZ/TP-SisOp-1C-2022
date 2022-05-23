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
	int* est_inicial;
	double* alfa;
	int* grad_multiprog;
	int* max_block;
} gralStruct;

config_conex* config_valores_kernel;
config_conex* config_valores_memoria;
config_conex* config_valores_cpu_dispatch;
config_conex* config_valores_cpu_interrupt;
//falta puerto escucha.

gralStruct valores_generales;


void load_configuration(){

	t_config* config = config_create("/home/utnso/workspace/tp-2022-1c-Messirve/kernel/cfg/kernel.config");


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.
	config_valores_kernel = malloc(sizeof(config_conex*));
	config_valores_kernel->ip = malloc(sizeof(char*));
	config_valores_kernel->ip = string_duplicate(config_get_string_value(config, "IP_KERNEL"));
	config_valores_kernel->puerto = config_get_int_value(config, "PUERTO_KERNEL");

	config_valores_memoria = malloc(sizeof(config_conex*));
	config_valores_memoria->ip = malloc(sizeof(char*));
	config_valores_memoria->ip = string_duplicate(config_get_string_value(config, "IP_MEMORIA"));
	config_valores_memoria->puerto = config_get_int_value(config, "PUERTO_MEMORIA");

	config_valores_cpu_dispatch = malloc(sizeof(config_conex*));
	config_valores_cpu_dispatch->ip = malloc(sizeof(char*));
	config_valores_cpu_dispatch->ip = string_duplicate(config_get_string_value(config, "IP_CPU_DISPATCH"));
	config_valores_cpu_dispatch->puerto = config_get_int_value(config, "PUERTO_CPU_DISPATCH");

	config_valores_cpu_interrupt = malloc(sizeof(config_conex*));
	config_valores_cpu_interrupt->ip = malloc(sizeof(char*));
	config_valores_cpu_interrupt->ip = string_duplicate(config_get_string_value(config, "IP_CPU_INTERRUPT"));
	config_valores_cpu_interrupt->puerto = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");

	//Lleno los struct de los campos que necesitamos para el pcb y demas.
	valores_generales = malloc(sizeof(gralStruct*));
	valores_generales->alg_planif = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	valores_generales->est_incial = config_get_int_value(config, "ESTIMACION_INICIAL");
	valores_generales->alfa = config_get_double_value(config, "ALFA");
	valores_generales->grad_multiprog = config_int_string_value(config, "GRADO_MULTIPROGRAMACION");
	valores_generales->max_bloc = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");

	//kernel_logger_info("IP_KERNEL: %s", config_valores->ip_kernel);
	//kernel_logger_info("PUERTO_KERNEL: %d", config_valores->puerto_kernel);
}

#endif /* SRC_UTILS_H_ */