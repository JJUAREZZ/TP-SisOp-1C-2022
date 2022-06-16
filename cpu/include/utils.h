#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "cpu.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>

t_log *logger;

uint32_t socket_dispatch;
uint32_t socket_interrupt;
uint32_t interrupcion;
sem_t semInterrupt;


typedef struct{
    int entradas_tlb;
    char* reemplazo_tlb;
    int retar_noop;
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    char* ip_cpu;
} valores_config_cpu;

valores_config_cpu *cpu_config;

void load_configuration_cpu(){
    sem_init(&semInterrupt,0,0);

	t_config* config = config_create("/home/utnso/workspace/tp-2022-1c-Messirve/cpu/cfg/cpu.config");

	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

    cpu_config=malloc(sizeof(valores_config_cpu));
	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.
    cpu_config->entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
    cpu_config->reemplazo_tlb =config_get_string_value(config, "REEMPLAZO_TLB");
    cpu_config->retar_noop = config_get_int_value(config, "RETARDO_NOOP");
    cpu_config->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    cpu_config->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    cpu_config->puerto_escucha_dispatch  = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    cpu_config->puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    cpu_config->ip_cpu = config_get_string_value(config, "IP_CPU");

}

float time_diff(struct timeval *start, struct timeval *end)
{
    int seconds  = end->tv_sec  - start->tv_sec;
    int useconds = end->tv_usec - start->tv_usec;

    float tiempo_dif = ((seconds) * 1000 + useconds/1000.0) + 0.5;
   // float tiempo_dif = (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
    return tiempo_dif;
}

#endif /* SRC_UTILS_H_ */