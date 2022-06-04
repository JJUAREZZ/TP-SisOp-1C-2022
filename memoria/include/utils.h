#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_
#include "../../shared/include/sockets.h"
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>

typedef struct{
    int puertoMemoria;
    int tamMemoria;
    int tamPagina;
    int pagPorTabla;
    int retardoMemoria;
    char* algoReemplazo;
    int marcPorProceso;
    int retardoSwap;
    char* pathSwap; 
} gralMemoria;

gralMemoria *valores_generales_memoria;

void load_configuration(){

	t_config* config = config_create("/home/utnso/workspace/tp-2022-1c-Messirve/memoria/cfg/memoria.config");


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

    valores_generales_memoria = malloc(sizeof(gralMemoria));
    valores_generales_memoria->puertoMemoria = config_get_string_value(config, "PUERTO_ESCUCHA");
    valores_generales_memoria->tamMemoria = config_get_int_value(config, "TAM_MEMORIA");
    valores_generales_memoria->tamPagina = config_get_int_value(config, "TAM_PAGINA");
    valores_generales_memoria->pagPorTabla = config_get_int_value(config, "PAGINAS_POR_TABLA");
    valores_generales_memoria->retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA");
    valores_generales_memoria->algoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    valores_generales_memoria->marcPorProceso = config_get_int_value(config, "MARCOS_POR_PROCESO");
    valores_generales_memoria->retardoSwap = config_get_int_value(config, "RETARDO_SWAP");
    valores_generales_memoria->pathSwap = config_get_string_value(config, "PATH_SWAP");

}







#endif /* SRC_MEMORIA_H_ */