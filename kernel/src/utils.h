/*
 * utils.h
 *
 *  Created on: 25 abr. 2022
 *      Author: utnso
 */

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
#include <commons/config.h>
#include <connect.h>
#include <pthread.h>

int kernel_socket;

typedef struct {
	char* ip_kernel;
	int puerto_kernel;
} kernel_config;

kernel_config* config_valores;

void load_configuration(){

	t_config* config = config_create("../kernel/kernel.config");


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	config_valores = malloc(sizeof(kernel_config*));
	config_valores->ip_kernel = malloc(sizeof(char*));
	config_valores->ip_kernel = string_duplicate(config_get_string_value(config, "IP_KERNEL"));
	config_valores->puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");

	kernel_logger_info("IP_KERNEL: %s", config_valores->ip_kernel);
	kernel_logger_info("PUERTO_KERNEL: %d", config_valores->puerto_kernel);
}

#endif /* SRC_UTILS_H_ */
