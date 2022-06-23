#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>

void *retornar_id_tabla_de_pagina(uint32_t);
void *atenderConexion(uint32_t );
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
		pthread_t hilo;
		uint32_t socket;
		socket= accept(memoria_socket,(struct sockaddr *) &client_info, &addrlen);
		if (socket != -1)
		{
			pthread_create(&hilo,NULL,atenderConexion,socket);
			
		}
		pthread_join(hilo,NULL);
	}
    return 0;
}

void *atenderConexion(uint32_t socket)
{
	pthread_t hilo;
	uint32_t cod_op= recibir_operacion(socket);
	if(cod_op>0)
	{
		switch (cod_op)
		{
		case TABLADEPAGINA:
			retornar_id_tabla_de_pagina(socket);

			//pthread_create(&hilo, NULL, retornar_id_tabla_de_pagina, socket);
			
			// SEGUN LA CATEDRA MEMORIA SOLO VA A TENER 2 HILOS UNO PARA ATENER A KERNEL Y OTRO PARA ATENDER A CPU
			// NO PODEMOS CREAR UN HILO POR CADA PETICION DISTINTA QUE SE QUIERA HACER (TABLAPAGINA, READ, WRITE, ETC)
			break;
		case READ:
			devolver_marco(socket);
		default:
			;
		}
	}
	//pthread_join(retornar_id_tabla_de_pagina,NULL);
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
	printf("\nArchivo swap del proceso Creado: %s \n", nombreArchivo);
	archivoswap = open(nombreArchivo, O_CREAT, O_RDWR);

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
		uint32_t id_tabla = crear_tabla_segundo_nivel(nro_paginas);
		tabla_primer_nivel->tablas_asociadas[i] = id_tabla;
	}
	list_add(tablas_primer_nivel_list, tabla_primer_nivel);

	return tabla_primer_nivel->id_primer_nivel;
}

uint32_t crear_tabla_segundo_nivel(uint32_t nro_paginas) 
{
	t_tabla_segundo_nivel *tabla_segundo_nivel = malloc(sizeof(t_tabla_segundo_nivel));
	tabla_segundo_nivel->id_segundo_nivel = id_tabla_segundo_nivel;
	id_tabla_segundo_nivel++;
	for(int i = 0; i < valores_generales_memoria->pagPorTabla; i++){
		t_paginas_en_tabla *pagina = crear_paginas(i);
		tabla_segundo_nivel->paginas[i] = pagina;
	}
	list_add(tablas_segundo_nivel_list, tabla_segundo_nivel);
	
	return tabla_segundo_nivel->id_segundo_nivel;
}

t_paginas_en_tabla *crear_paginas(uint32_t id) 
{
	t_paginas_en_tabla* pagina = malloc(sizeof(t_paginas_en_tabla));
	pagina->id_pagina = id;
	pagina->bit_presencia = 0;
	pagina->bit_uso = 0;
	pagina->bit_modificado = 0;
	pagina->marco = 1;

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
