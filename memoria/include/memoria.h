#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_
#include "../../shared/include/sockets.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

t_list tablasDePrimerNivel;
t_list tablasDeSegundoNivel;
t_bloque_memoria memPrincipal;

uint32_t id_tabla_primer_nivel;
uint32_t id_tabla_segundo_nivel;


//INICIALIZAR MEMORIA COMO SERVIDOR DE KERNEL Y CPU
memoria_init_server(){

}

void crear_memoria_principal(){

    memPrincipal.memoria_principal[valores_generales_memoria->tamMemoria]; //TODO malloc
    memPrincipal.bitmap_memoria = bitarray_create_with_mode("bitmap_memoria", valores_generales_memoria->tamMemoria, LSB_FIRST);
    printf("\n Memoria Principal Creada \n");
}















#endif /* SRC_MEMORIA_H_ */