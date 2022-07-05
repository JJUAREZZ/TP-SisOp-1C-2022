#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_
#include "../../shared/include/sockets.h"
#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

t_bloque_memoria *memPrincipal;

uint32_t id_tabla_primer_nivel;
uint32_t id_tabla_segundo_nivel;


//INICIALIZAR MEMORIA COMO SERVIDOR DE KERNEL Y CPU
memoria_init_server(){

}

void crear_memoria_principal(){

    uint32_t cantidad_de_marcos = valores_generales_memoria->tamMemoria / valores_generales_memoria->tamPagina;

    if(valores_generales_memoria->tamMemoria % valores_generales_memoria->tamPagina != 0){
        cantidad_de_marcos ++;
    }

    memPrincipal = malloc(sizeof(t_bloque_memoria));
    memPrincipal->memoria_principal[valores_generales_memoria->tamMemoria];
    memPrincipal->bitmap_memoria = bitarray_create_with_mode("bitmap_memoria",cantidad_de_marcos, MSB_FIRST);
    printf("\nMemoria Principal Creada\n");

}

















#endif /* SRC_MEMORIA_H_ */