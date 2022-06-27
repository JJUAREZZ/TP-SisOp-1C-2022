#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>


typedef struct{
	uint32_t *socket1;
	uint32_t *cod_op1;
}arg_struct;

void *retornar_id_tabla_de_pagina(uint32_t);
void *atenderConexionKernel(arg_struct*);
void *atenderConexionCpu(arg_struct*);
void *liberarProcesoDeMemoria(uint32_t);
void *liberarProcesoDeMemoriaYDeleteSwap(uint32_t );
uint32_t crear_tabla_del_proceso(pcb *unPcb);
uint32_t crear_tabla_segundo_nivel(uint32_t);
t_paginas_en_tabla *crear_paginas(uint32_t); 
uint32_t memoria_socket;
pcb *unPcb;
uint32_t *archivoswap;

t_list *tablas_primer_nivel_list;
t_list *tablas_segundo_nivel_list;

int main()
{
    load_configuration();
	crear_memoria_principal();

	tablas_primer_nivel_list = list_create();
	tablas_segundo_nivel_list = list_create();	

    struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	printf("Creando socket y escuchando \n");

	memoria_socket = socket_create_listener("127.0.0.1", valores_generales_memoria->puertoMemoria);

	if(memoria_socket < 0){
		printf("Error al crear server");
		return -1;
	}
	printf("¡¡¡Servidor de Memoria Iniciado!!!\n");
	for (;;) 
	{
		pthread_t hiloKernel;
		pthread_t hiloCpu;
		uint32_t socket;
		socket= accept(memoria_socket,(struct sockaddr *) &client_info, &addrlen);
		if (socket != -1)
		{
			uint32_t cod_op= recibir_operacion(socket);
			if(cod_op>0){
				if(cod_op == TABLADEPAGINA ||cod_op == DELETESWAP || cod_op == SUSPENDED){
				arg_struct argumentsK;
				argumentsK.socket1 = socket;
				argumentsK.cod_op1 = cod_op;
				pthread_create(&hiloKernel,NULL,atenderConexionKernel, &argumentsK);	
				pthread_join(hiloKernel, NULL);
				} 
				else if(cod_op == READ){
				arg_struct argumentsC;
				argumentsC.socket1 = socket;
				argumentsC.cod_op1 = cod_op;
				pthread_create(&hiloCpu, NULL, atenderConexionCpu, &argumentsC);
				pthread_join(hiloCpu,NULL);
				}
			}
		}
		
	
		
	}
    return 0;
}

void *atenderConexionKernel(arg_struct* argumentsK)
{
	//uint32_t cod_op= recibir_operacion(socket);
	//if(cod_op>0)
	//{
		uint32_t cod_op = argumentsK->cod_op1;
		uint32_t socket = argumentsK->socket1;
		switch (cod_op)
		{
		case TABLADEPAGINA:
			retornar_id_tabla_de_pagina(socket);
			break;
		case SUSPENDED: 
			liberarProcesoDeMemoria(socket);
			break;
		case DELETESWAP: 
			liberarProcesoDeMemoriaYDeleteSwap(socket);
			break;
		default:
			break;
		}
	//}
}

void *atenderConexionCpu(arg_struct* argumentsC){
	//uint32_t cod_op = recibir_operacion(socket);
	//if(cod_op > 0){
		uint32_t cod_op = argumentsC->cod_op1;
		uint32_t socket = argumentsC->socket1;
		switch(cod_op){
			case READ:
				devolver_marco(socket);
				break;
			default:
				break;		
		}
	//}
}



void *retornar_id_tabla_de_pagina(uint32_t socket)
{
	unPcb = recibir_pcb(socket);
	uint32_t idTabla = crear_tabla_del_proceso(unPcb);

	char* path = valores_generales_memoria->pathSwap;
	char nombreArchivo [50];

	//Creo el archivo swap.
	char* nroProceso [2];
	sprintf(nroProceso, "%d", unPcb->id);
	strcat(nroProceso, ".swap");
	strcat(strcpy(nombreArchivo, path), "/");
	strcat(nombreArchivo, nroProceso);
	archivoswap = open(nombreArchivo, O_CREAT, O_RDWR);
	printf("\nArchivo swap del proceso Creado: %s \n", nombreArchivo);

	memset(nombreArchivo, 0, strlen(nombreArchivo));
	memset(nroProceso, 0, strlen(nroProceso));

	if(archivoswap == -1){
		printf("Error al crear el archivo swap del proceso %n \n", unPcb->id);
		exit(1);
	}

	//Cargo el archivo de swap en 0;
	uint32_t cantidad_paginas_necesarias = unPcb->tamanioProceso / valores_generales_memoria->tamPagina;

	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0){
		cantidad_paginas_necesarias ++;
	}

	int cantidad_de_bytes = cantidad_paginas_necesarias * valores_generales_memoria->tamPagina;
	
	for(int i = 0; i<cantidad_de_bytes ; i++){
		write(archivoswap, 0, sizeof(uint8_t));
	}

	printf("\nRecibi un proceso: nroDeProceso= %d", unPcb->id);
	printf("\nAsignando tabla de pagina: id= %d\n",idTabla);
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&idTabla,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);
}




uint32_t crear_tabla_del_proceso(pcb *unPcb)
{
	t_tabla_primer_nivel *tabla_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));
	tabla_primer_nivel->id_primer_nivel = id_tabla_primer_nivel;
	id_tabla_primer_nivel++;
	uint32_t nro_paginas = unPcb->tamanioProceso / valores_generales_memoria->tamPagina;
	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0) nro_paginas++;
	uint32_t nro_tablas_segundo_nivel = nro_paginas / valores_generales_memoria->pagPorTabla;
	if(nro_paginas % valores_generales_memoria->pagPorTabla != 0) nro_tablas_segundo_nivel++;
	for(int i = 0; i < nro_tablas_segundo_nivel; i++){
		uint32_t id_tabla = crear_tabla_segundo_nivel(i);
		tabla_primer_nivel->tablas_asociadas[i] = id_tabla;
	}
	list_add(tablas_primer_nivel_list, tabla_primer_nivel);

	return tabla_primer_nivel->id_primer_nivel;
}

uint32_t crear_tabla_segundo_nivel(uint32_t entrada_primer_nivel) 
{
	t_tabla_segundo_nivel *tabla_segundo_nivel = malloc(sizeof(t_tabla_segundo_nivel));
	tabla_segundo_nivel->id_segundo_nivel = id_tabla_segundo_nivel;
	id_tabla_segundo_nivel++;
	for(int i = 0; i < valores_generales_memoria->pagPorTabla; i++){
		t_paginas_en_tabla *pagina = crear_paginas((entrada_primer_nivel * valores_generales_memoria->pagPorTabla) + i);
		tabla_segundo_nivel->paginas[i] = pagina;
	}
	list_add(tablas_segundo_nivel_list, tabla_segundo_nivel);
	
	return tabla_segundo_nivel->id_segundo_nivel;
}

t_paginas_en_tabla *crear_paginas(uint32_t id) 
{
	t_paginas_en_tabla* pagina = malloc(sizeof(t_paginas_en_tabla));
	pagina->id_pagina = id;
	//printf("\nId de pagina: %d\n", pagina->id_pagina);
	pagina->bit_presencia = 0;
	pagina->bit_uso = 0;
	pagina->bit_modificado = 0;
	pagina->marco = 0;

	return pagina;
}

void devolver_marco(uint32_t socket) 
{
	// arbitrariamente voy a la primer pagina de la primer tabla
	t_tabla_primer_nivel *tabla_primer_nivel = list_get(tablas_primer_nivel_list, 0);
	uint32_t id_tabla_segundo_level = tabla_primer_nivel->tablas_asociadas[0];
	t_tabla_segundo_nivel *tabla_segundo_nivel = list_get(tablas_segundo_nivel_list, id_tabla_segundo_level);
	t_paginas_en_tabla *pagina =  tabla_segundo_nivel->paginas[0];
	uint32_t marco = pagina->marco;

	printf("\nTe devuevlo el marco nro: %d\n", marco);
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&marco,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);

}

void *liberarProcesoDeMemoria(uint32_t socket){
	//Ver si recibir pcb o tabla de paginas.
	unPcb = recibir_pcb(socket);
	printf("\nProceso %d para liberar en memoria\n", unPcb->id);
	uint8_t *fd;
	uint8_t *addr;
	int i, j, k;

	//Archivo swap de este proceso especifico.
	char* path = valores_generales_memoria->pathSwap;
	char nombreArchivo [50];
	char* nroProceso [2];
	sprintf(nroProceso, "%d", unPcb->id);
	strcat(nroProceso, ".swap");
	strcat(strcpy(nombreArchivo, path), "/");
	strcat(nombreArchivo, nroProceso);

	uint32_t nro_paginas = unPcb->tamanioProceso/valores_generales_memoria->tamPagina;
	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0) nro_paginas++;

	fd = open(nombreArchivo, O_RDWR);
	uint8_t largoDelArchivo = valores_generales_memoria->pagPorTabla * valores_generales_memoria->pagPorTabla;

	//mapeo el archivo del proceso.
	addr = mmap(NULL, sizeof(uint8_t) * largoDelArchivo, PROT_WRITE, MAP_SHARED, fd, 0);

	//Busco la tabla de primer nivel.
    t_tabla_primer_nivel *primerNivel;
	primerNivel = list_get(tablas_primer_nivel_list, unPcb->tablaDePaginas);

	uint32_t nro_tablas_segundo_nivel = nro_paginas / valores_generales_memoria->pagPorTabla;
	if(nro_paginas % valores_generales_memoria->pagPorTabla != 0) nro_tablas_segundo_nivel++;

	//Itero por las tablas de segundo nivel.
	for(i = 0; i< nro_tablas_segundo_nivel; i++){
		t_tabla_segundo_nivel *segundoNivel;
		uint32_t id_segundo_nivel = primerNivel->tablas_asociadas[i];
		segundoNivel = list_get(tablas_segundo_nivel_list, id_segundo_nivel);

		//Itero por las paginas de la tabla de segundo nivel y veo si esta en uno su bit de presencia.
		for(j=0; j< valores_generales_memoria->pagPorTabla; j++){
			t_paginas_en_tabla *pagina;
			pagina = segundoNivel->paginas[j];

			//Bit de presencia en uno => desalojo esa pagina del marco en el que esta.
			if(pagina->bit_presencia == 1){

				uint8_t *comienzoDelMarco = *pagina->marco * (uint8_t)valores_generales_memoria->tamPagina;
				uint8_t *finDelMarco = *comienzoDelMarco + (uint8_t)valores_generales_memoria->tamPagina;

				//Saco la pagina del marco y la mando al swap correspondiente.
				uint8_t *IncioPagina = pagina->id_pagina * (uint8_t )valores_generales_memoria->tamPagina;
				uint8_t *finDeLaPagina = *IncioPagina + (uint8_t )valores_generales_memoria->tamPagina;

				//Copio el marco en el swap.
				memcpy(*addr + *IncioPagina, **memPrincipal->memoria_principal + *comienzoDelMarco, sizeof(uint8_t) * valores_generales_memoria->tamPagina);
				

				//Saco la pagina del marco y dejo el marco en 0.
				for(k = comienzoDelMarco; k < finDelMarco; k++){
					memPrincipal->memoria_principal[k] == 0;	
				}

				bitarray_set_bit(memPrincipal->bitmap_memoria, pagina->marco);
				pagina->bit_presencia = 0;

				printf("\n se ha liberado el espacio del proceso %n de memoria", unPcb->id);

			}
		}
	}

	printf("\n se ha liberado el espacio del proceso %d de memoria\n", unPcb->id);
	//ENVIAR MENSAJE A KERNEL CON PROCESO DESALOJADO
	uint32_t result = 1;
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&result,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);

}

void *liberarProcesoDeMemoriaYDeleteSwap(uint32_t socket){
	//Ver si recibir pcb o tabla de paginas.
    unPcb = recibir_pcb(socket);
	printf("\nProceso %d para liberar en memoria y eliminar swap\n", unPcb->id);
	uint8_t *fd;
	uint8_t *addr;
	int i, j, k;

	//Archivo swap de este proceso especifico.
	char* path = valores_generales_memoria->pathSwap;
	char nombreArchivo [50];
	char* nroProceso [2];
	sprintf(nroProceso, "%d", unPcb->id);
	strcat(nroProceso, ".swap");
	strcat(strcpy(nombreArchivo, path), "/");
	strcat(nombreArchivo, nroProceso);

	uint32_t nro_paginas = unPcb->tamanioProceso/valores_generales_memoria->tamPagina;
	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0) nro_paginas++;

	uint32_t nro_tablas_segundo_nivel = nro_paginas / valores_generales_memoria->pagPorTabla;
	if(nro_paginas % valores_generales_memoria->pagPorTabla != 0) nro_tablas_segundo_nivel++;

	remove(nombreArchivo);

	//Busco la tabla de primer nivel.
    t_tabla_primer_nivel *primerNivel;
	primerNivel = list_get(tablas_primer_nivel_list, unPcb->tablaDePaginas);

	//Itero por las tablas de segundo nivel.
	for(i = 0; i< nro_tablas_segundo_nivel; i++){
		t_tabla_segundo_nivel *segundoNivel;
		uint32_t id_segundo_nivel = primerNivel->tablas_asociadas[i];
		segundoNivel = list_get(tablas_segundo_nivel_list, id_segundo_nivel);

		//Itero por las paginas de la tabla de segundo nivel y veo si esta en uno su bit de presencia.
		for(j=0; j< valores_generales_memoria->pagPorTabla; j++){
			t_paginas_en_tabla *pagina;
			pagina = segundoNivel->paginas[j];

			//Bit de presencia en uno => desalojo esa pagina del marco en el que esta.
			if(pagina->bit_presencia == 1){

				uint8_t *comienzoDelMarco = *pagina->marco * (uint8_t)valores_generales_memoria->tamPagina;
				uint8_t *finDelMarco = *comienzoDelMarco + (uint8_t)valores_generales_memoria->tamPagina;

				//Saco la pagina del marco y la mando al swap correspondiente.
				uint8_t *IncioPagina = pagina->id_pagina * (uint8_t )valores_generales_memoria->tamPagina;
				uint8_t *finDeLaPagina = *IncioPagina + (uint8_t )valores_generales_memoria->tamPagina;
		
				//Saco la pagina del marco y dejo el marco en 0.
				for(k = comienzoDelMarco; k < finDelMarco; k++){
					memPrincipal->memoria_principal[k] == 0;	
				}

				bitarray_set_bit(memPrincipal->bitmap_memoria, pagina->marco);
				pagina->bit_presencia = 0;

			}
		}
	}

	printf("\n se ha eliminado el proceso %d de memoria.\n", unPcb->id);

	uint32_t result = 1;
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&result,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);

}