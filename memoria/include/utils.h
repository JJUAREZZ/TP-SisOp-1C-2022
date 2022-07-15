#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_
#include "../../shared/include/sockets.h"
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/log.h>
#include <pthread.h>

t_log *logger;

typedef struct{
    char* puertoMemoria;
    int tamMemoria;
    int tamPagina;
    int pagPorTabla;
    int retardoMemoria;
    char* algoReemplazo;
    int marcPorProceso;
    int retardoSwap;
    char* pathSwap; 
    char* ipMemoria;
} gralMemoria;

t_bitarray *bitmap_memoria;
void *memoria_principal;
void *puntero_a_bits;

typedef struct{
    uint32_t id_primer_nivel;
    t_queue *paginas_en_memoria;
    uint32_t *tablas_asociadas;
    uint32_t index;
} t_tabla_primer_nivel;

typedef struct{
    uint32_t id_pagina;
    uint32_t  marco;
    uint32_t bit_presencia; 
    uint32_t bit_uso;
    uint32_t bit_modificado;
} t_paginas_en_tabla;

typedef struct{
    uint32_t id_segundo_nivel;
    t_paginas_en_tabla *paginas;
} t_tabla_segundo_nivel;

gralMemoria *valores_generales_memoria;

char *path_memoria_config;

void load_configuration(){

	t_config *config = config_create(path_memoria_config);

	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

    valores_generales_memoria = malloc(sizeof(gralMemoria));
    valores_generales_memoria->puertoMemoria = config_get_string_value(config, "PUERTO_ESCUCHA");
    valores_generales_memoria->tamMemoria = config_get_int_value(config, "TAM_MEMORIA");
    valores_generales_memoria->tamPagina = config_get_int_value(config, "TAM_PAGINA");
    valores_generales_memoria->pagPorTabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    valores_generales_memoria->retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA");
    valores_generales_memoria->algoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    valores_generales_memoria->marcPorProceso = config_get_int_value(config, "MARCOS_POR_PROCESO");
    valores_generales_memoria->retardoSwap = config_get_int_value(config, "RETARDO_SWAP");
    valores_generales_memoria->pathSwap= config_get_string_value(config, "PATH_SWAP");
    valores_generales_memoria->ipMemoria = "127.0.0.1";

    //config_destroy(config);
}

#endif /* SRC_MEMORIA_H_ */