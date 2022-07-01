#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>


typedef struct{
	uint32_t *socket1;
	uint32_t *cod_op1;
}arg_struct;

void *conectarse_con_kernel(uint32_t );
void *conectarse_con_cpu(uint32_t );

void *handshake(uint32_t);
void *retornar_id_tabla_de_pagina(uint32_t);
void *atenderConexionKernel(arg_struct*);
void *atenderConexionCpu(arg_struct*);
void *liberarProcesoDeMemoria(uint32_t);
void *liberarProcesoDeMemoriaYDeleteSwap(uint32_t );
uint32_t crear_tabla_del_proceso(pcb *unPcb);
uint32_t crear_tabla_segundo_nivel(uint32_t);
t_paginas_en_tabla *crear_paginas(uint32_t); 
void* devolver_id_tabla_segundo_nivel(uint32_t);
void* devolver_marco(uint32_t);
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

	pthread_t conexion_con_kernel, conexion_con_cpu;
	uint32_t socket_cpu, socket_kernel;

	socket_cpu= accept(memoria_socket,(struct sockaddr *) &client_info, &addrlen);
	if(socket_cpu>0)
		pthread_create(&conexion_con_cpu,NULL,conectarse_con_cpu,socket_cpu);

	socket_kernel= accept(memoria_socket,(struct sockaddr *) &client_info, &addrlen);
	if(socket_kernel>0)
		pthread_create(&conexion_con_kernel,NULL,conectarse_con_kernel,socket_kernel);
	
	pthread_join(conexion_con_kernel, NULL);
	pthread_join(conexion_con_cpu, NULL);	
}


void *handshake(uint32_t socket){
	void *buffer;
	
	uint32_t cod_op= HANDSHAKE_CPU;
	uint32_t offset=0;
	uint32_t tamanio= sizeof(uint32_t)*3;
	buffer= malloc(tamanio);
	memcpy(buffer,&cod_op,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&valores_generales_memoria->tamPagina,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset,&valores_generales_memoria->pagPorTabla,sizeof(uint32_t));
	
	send(socket, buffer, tamanio, 0);

	free(buffer);
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
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	uint32_t cod_op= TABLADEPAGINA;
	memcpy(stream, &cod_op,sizeof(uint32_t));
	memcpy(stream+sizeof(uint32_t),&idTabla,sizeof(uint32_t));
	send(socket,stream,tamanio,NULL);
	free(stream);
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
				usleep(valores_generales_memoria->retardoSwap);
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

	printf("\nSe ha liberado el espacio del proceso %d de memoria\n", unPcb->id);
	//ENVIAR MENSAJE A KERNEL CON PROCESO DESALOJADO
	uint32_t result = 1;
	uint32_t cod_op= SUSPENDED;
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	memcpy(stream,&cod_op,sizeof(uint32_t));
	memcpy(stream+ sizeof(uint32_t),&result,sizeof(uint32_t));
	send(socket,stream,tamanio,NULL);
	free(stream);

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

	printf("\nSe ha eliminado el proceso %d de memoria.\n", unPcb->id);

	uint32_t result = 1;
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	uint32_t cod_op= DELETESWAP;
	memcpy(stream,&cod_op,sizeof(uint32_t));
	memcpy(stream+sizeof(uint32_t),&result,sizeof(uint32_t));
	send(socket,stream,tamanio,NULL);
	free(stream);

}

void* devolver_id_tabla_segundo_nivel(uint32_t socket){
	uint32_t entrada_tabla_1er_nivel,id_tabla_primer_nivel, cod_op;
	//recv(socket, &cod_op, sizeof(uint32_t), 0);
	recv(socket, &id_tabla_primer_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &entrada_tabla_1er_nivel, sizeof(uint32_t), MSG_WAITALL);	

	t_tabla_primer_nivel *tabla;
	tabla = list_get(tablas_primer_nivel_list, id_tabla_primer_nivel);
	uint32_t id_tabla_segundo_nivel = tabla->tablas_asociadas[entrada_tabla_1er_nivel];
	
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&id_tabla_segundo_nivel,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);

	printf("\nEntrada %d tabla de segundo nivel enviada con exito.\n", id_tabla_segundo_nivel);

	free(stream);
}

void* devolver_marco(uint32_t socket){
	uint32_t tabla_primer_nivel,tabla_segundo_nivel, entrada_segundo_nivel, cod_op;

	//recv(socket, &cod_op, sizeof(uint32_t), 0);
	recv(socket, &tabla_primer_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &tabla_segundo_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &entrada_segundo_nivel, sizeof(uint32_t), MSG_WAITALL);

	t_tabla_primer_nivel *tabla1;
	tabla1 = list_get(tablas_primer_nivel_list, tabla_primer_nivel);

	t_tabla_segundo_nivel *tabla;
	tabla = list_get(tablas_segundo_nivel_list, tabla_segundo_nivel);

	t_paginas_en_tabla *pagina;
	pagina = tabla->paginas[entrada_segundo_nivel];
	
	if(pagina->bit_presencia == 1){
		uint32_t *marco = pagina->marco;

		void *stream= malloc(sizeof(uint32_t));
		memcpy(stream,&marco,sizeof(uint32_t));
		send(socket,stream,sizeof(uint32_t),NULL);

		printf("\nMarco %d enviado al MMU.\n", marco);

		free(stream);

	} else if(pagina -> bit_presencia != 0) {

		int utilizarClock = strcmp(valores_generales_memoria->algoReemplazo, "CLOCK");
		int utilizarClockM = strcmp(valores_generales_memoria->algoReemplazo, "CLOCK-M");

		printf("\n La pagina %d no se encuentra cargada en memoria\n", pagina->id_pagina);

		// Recorrer las tablas de paginas.
		size_t *cant_primeras_entradas = sizeof(&tabla1->tablas_asociadas) / sizeof(tabla1->tablas_asociadas[0]); 

		uint32_t cant_marcos_asignados;
		cant_marcos_asignados = 0;

		for(int i = 0; i<cant_primeras_entradas ; i++){
			t_tabla_segundo_nivel *tablaSeg = list_get(tablas_segundo_nivel_list, tabla1->tablas_asociadas[i]);
			for(int j=0; j<valores_generales_memoria->pagPorTabla; j++){
				t_paginas_en_tabla *tPagina = tablaSeg->paginas[j];
				if(tPagina->bit_presencia == 1);
				cant_marcos_asignados ++;
			}
		}

		//Si la cantidad de marcos asignados es == marcos por proceso -> Realizo algoritmo reemplazo.
		if(cant_marcos_asignados == valores_generales_memoria->marcPorProceso){
				if(utilizarClock == 0){
			//Desarrollo algoritmo Clock.
			}	
			if(utilizarClockM == 0){
			//Desarrollo algoritmo ClockM.
			}
		} else if (cant_marcos_asignados < valores_generales_memoria->marcPorProceso){
			//TODO: leer de swap y asignar alguna pagina al marco.
		}	
	}

}

void *conectarse_con_kernel(uint32_t socket){
	while(1){
		uint32_t cod_op= recibir_operacion(socket);
		if(cod_op>0){
			switch (cod_op){
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
		}
	}
}

void *conectarse_con_cpu(uint32_t socket){
	while(1){
		uint32_t cod_op= recibir_operacion(socket);
		if(cod_op>0)
		{
			switch(cod_op){
			case HANDSHAKE_CPU:
				handshake(socket);
				break;
			case READ:
				devolver_marco(socket);
				break;
			case IDTABLASEGUNDONIVEL:
				devolver_id_tabla_segundo_nivel(socket);
				break;
			case MARCO:
				devolver_marco(socket);
				break;
			default:
				break;		
			}
		}
	}
}

