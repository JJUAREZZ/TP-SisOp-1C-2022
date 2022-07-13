#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>


typedef struct{
	uint32_t *socket1;
	uint32_t *cod_op1;
}arg_struct;

void *conectarse_con_kernel(uint32_t );
void *conectarse_con_cpu(uint32_t );
void *devolver_valor_memoria(uint32_t );
void escribir_en_memoria(uint32_t);
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

int main(int argc, char** argv) {
	path_memoria_config = (char*) argv[1];
    load_configuration();
	logger = log_create("log.log", "Servidor Memoria", 1, LOG_LEVEL_INFO);
	crear_memoria_principal();
	tablas_primer_nivel_list = list_create();
	tablas_segundo_nivel_list = list_create();

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	log_info(logger, "Creando socket y escuchando \n");

	memoria_socket = socket_create_listener(valores_generales_memoria->ipMemoria, valores_generales_memoria->puertoMemoria);
	
	if(memoria_socket < 0){
		log_info(logger, "Error al crear server");
		return -1;
	}
	log_info(logger,"¡¡¡Servidor de Memoria Iniciado!!!\n");

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
	
	usleep(valores_generales_memoria->retardoMemoria * 1000);
	send(socket, buffer, tamanio, 0);

	free(buffer);
}


void *retornar_id_tabla_de_pagina(uint32_t socket)
{
	unPcb = recibir_pcb(socket);
	uint32_t idTabla = crear_tabla_del_proceso(unPcb);

	char* path = valores_generales_memoria->pathSwap;
	char nombreArchivo [50];
	uint32_t *addr;
	//Creo el archivo swap.
	char* nroProceso [2];
	sprintf(nroProceso, "%d", unPcb->id);
	strcat(nroProceso, ".swap");
	strcat(strcpy(nombreArchivo, path), "/");
	strcat(nombreArchivo, nroProceso);

	mkdir(path, 0775);

	archivoswap = open(nombreArchivo,O_CREAT | O_RDWR, S_IRWXU);
	log_info(logger, "\nArchivo swap del proceso Creado: %s \n", nombreArchivo);

	memset(nombreArchivo, 0, strlen(nombreArchivo));
	memset(nroProceso, 0, strlen(nroProceso));

	if(archivoswap == -1){
		log_info(logger,"Error al crear el archivo swap del proceso %n \n", unPcb->id);
		exit(1);
	}

	//Cargo el archivo de swap en 0;
	uint32_t cantidad_paginas_necesarias = unPcb->tamanioProceso / valores_generales_memoria->tamPagina;

	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0){
		cantidad_paginas_necesarias ++;
	}

	int cantidad_de_bytes = cantidad_paginas_necesarias * valores_generales_memoria->tamPagina;

	size_t tamanioArchivo = cantidad_de_bytes;

	uint32_t nCero = 0;

	for(int i = 0; i<cantidad_de_bytes; i++){
		write(archivoswap, "0", 1);
	}

	struct stat fd_size;

	fstat(archivoswap, &fd_size);

	addr = mmap(NULL,fd_size.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, archivoswap, 0);
	
	if(addr == MAP_FAILED){
		log_error(logger, "Error mapping\n");
	}

	close(archivoswap);

	log_info(logger, "\nRecibi un proceso: nroDeProceso= %d", unPcb->id);
	log_info(logger,"\nAsignando tabla de pagina: id= %d\n",idTabla);
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	uint32_t cod_op= TABLADEPAGINA;
	memcpy(stream, &cod_op,sizeof(uint32_t));
	memcpy(stream+sizeof(uint32_t),&idTabla,sizeof(uint32_t));

	usleep(valores_generales_memoria->retardoMemoria * 1000);

	send(socket,stream,tamanio,NULL);
	liberarPcb(unPcb);
	free(stream);
}


uint32_t crear_tabla_del_proceso(pcb *unPcb)
{
	t_tabla_primer_nivel *tabla_primer_nivel = malloc(sizeof(t_tabla_primer_nivel));
	tabla_primer_nivel->id_primer_nivel = id_tabla_primer_nivel;
	id_tabla_primer_nivel++;

	uint32_t nro_paginas = unPcb->tamanioProceso / valores_generales_memoria->tamPagina;
	if(unPcb->tamanioProceso % valores_generales_memoria->tamPagina != 0)
		nro_paginas++;
	uint32_t nro_tablas_segundo_nivel = nro_paginas / valores_generales_memoria->pagPorTabla;
	if(nro_paginas % valores_generales_memoria->pagPorTabla != 0)
		nro_tablas_segundo_nivel++;

	tabla_primer_nivel->tablas_asociadas= malloc(sizeof(uint32_t)*nro_tablas_segundo_nivel);

	for(int i = 0; i < nro_tablas_segundo_nivel; i++){
		uint32_t id_tabla = crear_tabla_segundo_nivel(i);
		memcpy(tabla_primer_nivel->tablas_asociadas+i,&id_tabla,sizeof(uint32_t));
	}
	list_add(tablas_primer_nivel_list, tabla_primer_nivel);
	tabla_primer_nivel->paginas_en_memoria = queue_create();
	tabla_primer_nivel->index = 0;

	return tabla_primer_nivel->id_primer_nivel;
}

uint32_t crear_tabla_segundo_nivel(uint32_t entrada_primer_nivel) 
{
	t_tabla_segundo_nivel *tabla_segundo_nivel = malloc(sizeof(t_tabla_segundo_nivel));
	tabla_segundo_nivel->paginas = malloc(sizeof(t_paginas_en_tabla)*valores_generales_memoria->pagPorTabla);
	tabla_segundo_nivel->id_segundo_nivel = id_tabla_segundo_nivel;
	id_tabla_segundo_nivel++;
	for(int i = 0; i < valores_generales_memoria->pagPorTabla; i++){
		uint32_t pid = (entrada_primer_nivel * valores_generales_memoria->pagPorTabla) + i ;
		t_paginas_en_tabla *pagina = crear_paginas(pid);
		memcpy(tabla_segundo_nivel->paginas+i,pagina,sizeof(t_paginas_en_tabla));
		free(pagina);
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

void* devolver_id_tabla_segundo_nivel(uint32_t socket){
	uint32_t entrada_tabla_1er_nivel,id_tabla_primer_nivel, cod_op;
	recv(socket, &id_tabla_primer_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &entrada_tabla_1er_nivel, sizeof(uint32_t), MSG_WAITALL);	

	t_tabla_primer_nivel *tabla;
	tabla = list_get(tablas_primer_nivel_list, id_tabla_primer_nivel);
	uint32_t id_tabla_segundo_nivel = *(tabla->tablas_asociadas+entrada_tabla_1er_nivel);
	
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&id_tabla_segundo_nivel,sizeof(uint32_t));

	usleep(valores_generales_memoria->retardoMemoria * 1000);

	send(socket,stream,sizeof(uint32_t),NULL);

	log_info(logger,"\nId %d tabla de segundo nivel enviada con exito.\n", id_tabla_segundo_nivel);

	free(stream);
}



void* devolver_marco(uint32_t socket){

	int w;

	uint32_t tabla_primer_nivel,tabla_segundo_nivel, entrada_segundo_nivel, cod_op;

	recv(socket, &tabla_primer_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &tabla_segundo_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &entrada_segundo_nivel, sizeof(uint32_t), MSG_WAITALL);

	t_tabla_primer_nivel *tabla1;
	tabla1 = list_get(tablas_primer_nivel_list, tabla_primer_nivel);

	t_tabla_segundo_nivel *tabla;
	tabla = list_get(tablas_segundo_nivel_list, tabla_segundo_nivel);

	t_paginas_en_tabla *pagina;
	pagina = tabla->paginas+entrada_segundo_nivel;

	int *fd;
	
	char* path = valores_generales_memoria->pathSwap;
	char nombreArchivo [50];
	char* nroProceso [2];
	sprintf(nroProceso, "%d", tabla_primer_nivel);
	strcat(nroProceso, ".swap");
	strcat(strcpy(nombreArchivo, path), "/");
	strcat(nombreArchivo, nroProceso);
	
	fd = open(nombreArchivo, O_RDWR, S_IRWXU);

	if(fd == -1){
		log_error(logger,"Error al abrir el archivo");
	}

	memset(nombreArchivo, 0, strlen(nombreArchivo));
	memset(nroProceso, 0, strlen(nroProceso));

	if(pagina->bit_presencia == 1){
		uint32_t marco = pagina->marco;

		void *stream= malloc(sizeof(uint32_t));
		memcpy(stream,&marco,sizeof(uint32_t));

		usleep(valores_generales_memoria->retardoMemoria * 1000);

		send(socket,stream,sizeof(uint32_t),NULL);

		log_info(logger,"\nMarco %d enviado al MMU.\n", marco);

		free(stream);

	} else if(pagina->bit_presencia == 0) {

		
		int utilizarClock = strcmp(valores_generales_memoria->algoReemplazo, "CLOCK");
		int utilizarClockM = strcmp(valores_generales_memoria->algoReemplazo, "CLOCK-M");

		log_info(logger,"\nLa pagina %d no se encuentra cargada en memoria\n", pagina->id_pagina);

		// Recorrer las tablas de paginas.
		size_t tamanioTabla = sizeof(&tabla1->tablas_asociadas);
		size_t tamanioEntrada = sizeof(&tabla1->tablas_asociadas);

		size_t cant_primeras_entradas = tamanioTabla / tamanioEntrada; 

		int cant_marcos_asignados;
		//cant_marcos_asignados = 0;

		//Esto se puede reemplazar por: 
		cant_marcos_asignados = queue_size(tabla1->paginas_en_memoria);

		//Si la cantidad de marcos asignados es == marcos por proceso -> Realizo algoritmo reemplazo.
		if(cant_marcos_asignados == valores_generales_memoria->marcPorProceso){

		if(utilizarClock == 0){
			//Algoritmo Clock.
			log_info(logger, "Utilizando algoritmo clock\n");
			t_queue *cola= tabla1->paginas_en_memoria;
			int size = queue_size(cola);
			t_list *lista = tabla1->paginas_en_memoria->elements;
	
			while(tabla1->index < size){
			t_paginas_en_tabla *pagina_a_reemplazar = list_get(lista, tabla1->index);
			
			if(pagina_a_reemplazar->bit_uso == 0){ //encontre a la victima

				//La busco en la tabla de paginas y le pongo el bit de presencia en 0.
				uint32_t primer_entrada = pagina_a_reemplazar->id_pagina / valores_generales_memoria->pagPorTabla;
				int id_tabla2 = *(tabla1->tablas_asociadas+primer_entrada);
				t_tabla_segundo_nivel *tabla2 = list_get(tablas_segundo_nivel_list, id_tabla2);
				int entrada2 = pagina_a_reemplazar->id_pagina - (primer_entrada * valores_generales_memoria->pagPorTabla);
				t_paginas_en_tabla *pagina_r_tabla = tabla2->paginas+entrada2;
				pagina_r_tabla->bit_presencia = 0;

				if(pagina_a_reemplazar->bit_modificado == 1){

				//Swapearla. 
					uint32_t comienzo_marco = pagina_a_reemplazar->marco * valores_generales_memoria->tamPagina;

					uint32_t comienzo_pagina_reemplazo = pagina_a_reemplazar->id_pagina * valores_generales_memoria->tamPagina;
					uint32_t inicio_pagina_nueva =pagina->id_pagina * valores_generales_memoria->tamPagina;
					//Saco la pagina del marco y la mando al swap correspondiente.
					
					//copio la pagina reemplazada en el swap.
					lseek(fd, comienzo_pagina_reemplazo, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);
					write(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);

					//Copio en la memoria la pagina nueva.
					lseek(fd, inicio_pagina_nueva, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);//multiplicarlo x 1000
					read(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);
					
				}
						
				pagina->marco = pagina_a_reemplazar->marco;
				pagina->bit_presencia = 1;
				list_replace(lista, tabla1->index , pagina);
				log_info(logger,"\nPagina %d reemplazada por la pagina %d en el marco %d.\n", pagina_a_reemplazar->id_pagina, pagina->id_pagina, pagina_a_reemplazar->marco);
				tabla1->index++; //avanzo al siguiente

				if(tabla1->index == size){
					tabla1->index=0;
				}

				void *stream= malloc(sizeof(uint32_t));
				memcpy(stream,&(pagina->marco),sizeof(uint32_t));

				usleep(valores_generales_memoria->retardoMemoria * 1000);

				send(socket,stream,sizeof(uint32_t),NULL);

				log_info(logger,"\nMarco %d enviado al MMU.\n", pagina->marco);

				free(stream);

				break;
			}

			if(pagina_a_reemplazar->bit_uso ==1){
				pagina_a_reemplazar->bit_uso = 0;
			}

			tabla1->index++; //avanzo al siguiente
			if(tabla1->index == size){
				tabla1->index = 0;
				continue;
				}
				}
		}		

		bool bitUsoYModCero(t_paginas_en_tabla* pagina_en_tabla){
			return (pagina_en_tabla->bit_uso == 0 && pagina_en_tabla->bit_modificado == 0);
		}

		if(utilizarClockM == 0){
			//Algoritmo ClockM.
			log_info(logger, "Utilizando algoritmo clock modificado\n");
			t_queue *cola = tabla1->paginas_en_memoria;
			int size = queue_size(cola);
			t_list *lista = tabla1->paginas_en_memoria->elements;

			//avanzo al siguiente
			while(tabla1->index < size){
			t_paginas_en_tabla *pagina_a_reemplazar = list_get(lista, tabla1->index);

			//a
			if(pagina_a_reemplazar->bit_uso ==0 && pagina_a_reemplazar->bit_modificado ==0){

				//La busco en la tabla de paginas y le pongo el bit de presencia en 0.
				uint32_t primer_entrada = pagina_a_reemplazar->id_pagina / valores_generales_memoria->pagPorTabla;
				int id_tabla2 = *(tabla1->tablas_asociadas+primer_entrada);
				t_tabla_segundo_nivel *tabla2 = list_get(tablas_segundo_nivel_list, id_tabla2);
				int entrada2 = pagina_a_reemplazar->id_pagina - (primer_entrada * valores_generales_memoria->pagPorTabla);
				t_paginas_en_tabla *pagina_r_tabla = tabla2->paginas+entrada2;
				pagina_r_tabla->bit_presencia = 0;

				if(pagina_a_reemplazar->bit_modificado == 1){

					//Saco la pagina del marco y la mando al swap correspondiente.
					uint32_t comienzo_marco = pagina_a_reemplazar->marco * valores_generales_memoria->tamPagina;

					uint32_t comienzo_pagina_reemplazo = pagina_a_reemplazar->id_pagina * valores_generales_memoria->tamPagina;
					uint32_t inicio_pagina_nueva = pagina->id_pagina * valores_generales_memoria->tamPagina;
					//Saco la pagina del marco y la mando al swap correspondiente.

					//copio la pagina reemplazada en el swap.
					lseek(fd, comienzo_pagina_reemplazo, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);
					write(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);

					//Copio en la memoria la pagina nueva.
					lseek(fd, inicio_pagina_nueva, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);//multiplicarlo x 1000
					read(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);
				}
		
				pagina->marco = pagina_a_reemplazar->marco;
				pagina->bit_presencia = 1;
				list_replace(lista, tabla1->index , pagina);
				log_info(logger,"\nPagina %d reemplazada por la pagina %d en el marco %d.\n", pagina_a_reemplazar->id_pagina, pagina->id_pagina, pagina_a_reemplazar->marco);
				tabla1->index++; 


				if(tabla1->index == size){
					tabla1->index=0;
				}

				void *stream= malloc(sizeof(uint32_t));
				memcpy(stream,&(pagina->marco),sizeof(uint32_t));

				usleep(valores_generales_memoria->retardoMemoria * 1000);

				send(socket,stream,sizeof(uint32_t),NULL);

				log_info(logger,"\nMarco %d enviado al MMU.\n", pagina->marco);

				free(stream);
			

				break;
			}
			
			if((list_any_satisfy(lista, bitUsoYModCero)) == 0){//no hay00

				if(pagina_a_reemplazar->bit_uso == 0 && pagina_a_reemplazar->bit_modificado == 1){
				
				//La busco en la tabla de paginas y le pongo el bit de presencia en 0.
				int primer_entrada = pagina_a_reemplazar->id_pagina / valores_generales_memoria->pagPorTabla;
				uint32_t id_tabla2 = *(tabla1->tablas_asociadas+primer_entrada);
				t_tabla_segundo_nivel *tabla2 = list_get(tablas_segundo_nivel_list, id_tabla2);
				uint32_t entrada2= pagina_a_reemplazar->id_pagina - (primer_entrada*valores_generales_memoria->pagPorTabla);
				t_paginas_en_tabla *pagina_r_tabla = tabla2->paginas+entrada2;
				pagina_r_tabla->bit_presencia = 0;

				if(pagina_a_reemplazar->bit_modificado == 1){
					uint32_t comienzo_marco = pagina_a_reemplazar->marco * valores_generales_memoria->tamPagina;

					uint32_t comienzo_pagina_reemplazo = pagina_a_reemplazar->id_pagina * valores_generales_memoria->tamPagina;
					uint32_t inicio_pagina_nueva = pagina->id_pagina * valores_generales_memoria->tamPagina;
					//Saco la pagina del marco y la mando al swap correspondiente.

					//copio la pagina reemplazada en el swap.
					lseek(fd, comienzo_pagina_reemplazo, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);
					write(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);

					//Copio en la memoria la pagina nueva.
					lseek(fd, inicio_pagina_nueva, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);//multiplicarlo x 1000
					read(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);									
				
				}
		
						
				pagina->marco = pagina_a_reemplazar->marco;
				pagina->bit_presencia = 1;
				list_replace(lista, tabla1->index , pagina);
				log_info(logger,"\nPagina %d reemplazada por la pagina %d en el marco %d.\n", pagina_a_reemplazar->id_pagina, pagina->id_pagina, pagina_a_reemplazar->marco);
				tabla1->index++; 

				if(tabla1->index == size){
					tabla1->index=0;
				}
				
				void *stream= malloc(sizeof(uint32_t));
				memcpy(stream,&(pagina->marco),sizeof(uint32_t));

				usleep(valores_generales_memoria->retardoMemoria * 1000);

				send(socket,stream,sizeof(uint32_t),NULL);

				log_info(logger,"\nMarco %d enviado al MMU.\n", pagina->marco);

				free(stream);

				break;

				}

			bool tieneUsoYMod = (pagina_a_reemplazar->bit_uso == 1 && pagina_a_reemplazar->bit_modificado == 1);
			bool tieneUso = pagina_a_reemplazar->bit_uso == 1;
			
			if(tieneUsoYMod | tieneUso){
			pagina_a_reemplazar->bit_uso = 0;
			}

			}
			
			tabla1->index ++; 
			if(tabla1->index == size){
				tabla1->index = 0;
				continue;
				}
			
			}

		}

		} else if (cant_marcos_asignados < valores_generales_memoria->marcPorProceso){
			//leer de swap y asignar alguna pagina al marco.
			uint32_t cantidad_marcos_memoria = valores_generales_memoria->tamMemoria / valores_generales_memoria->tamPagina;
			
			//Busco Marco libre.
			
			for(int w=0; w<cantidad_marcos_memoria; w++){
				bool marco_ocupado = bitarray_test_bit(bitmap_memoria, w);
		
				if(marco_ocupado == false){
					//Si el marco esta libre se lo asigno a la pagina y hago el swap.
					bitarray_set_bit(bitmap_memoria, w);
					msync(bitmap_memoria->bitarray, tamanioBitmap, MS_SYNC);

					pagina->marco = w;

					uint32_t comienzo_marco = pagina->marco * valores_generales_memoria->tamPagina;
					uint32_t comienzo_pagina = pagina->id_pagina * valores_generales_memoria->tamPagina;

					//Hago el swap.
					lseek(fd, comienzo_pagina, SEEK_SET);
					usleep(valores_generales_memoria->retardoSwap * 1000);//multiplicarlo x 1000
					read(fd, memoria_principal+comienzo_marco, valores_generales_memoria->tamPagina);
				
					pagina->bit_presencia = 1;
					pagina->bit_uso = 1;

					queue_push(tabla1->paginas_en_memoria, pagina);

					log_info(logger,"Marco %d asignado a la pagina %d", w, pagina->id_pagina);

					void *stream= malloc(sizeof(uint32_t));
					memcpy(stream,&(pagina->marco),sizeof(uint32_t));

					usleep(valores_generales_memoria->retardoMemoria * 1000);

					send(socket,stream,sizeof(uint32_t),NULL);

					log_info(logger,"\nMarco %d enviado al MMU.\n", pagina->marco);

					free(stream);

					break;
				}
			}

		}	
	}

	close(fd);
}

void *liberarProcesoDeMemoria(uint32_t socket){
	unPcb = recibir_pcb(socket);
	log_info(logger,"\nProceso %d para liberar en memoria\n", unPcb->id);
	int *fd;
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


	//mapeo el archivo del proceso.
	fd = open(nombreArchivo, O_RDWR, S_IRWXU);

	if(fd == -1){
		log_error(logger,"Error al abrir el archivo.");
	}

	memset(nombreArchivo, 0, strlen(nombreArchivo));
	memset(nroProceso, 0, strlen(nroProceso));

	//Busco la tabla de primer nivel.
    t_tabla_primer_nivel *primerNivel;
	primerNivel = list_get(tablas_primer_nivel_list, unPcb->tablaDePaginas);

	uint32_t nro_tablas_segundo_nivel = nro_paginas / valores_generales_memoria->pagPorTabla;
	if(nro_paginas % valores_generales_memoria->pagPorTabla != 0) nro_tablas_segundo_nivel++;

	//Itero por las tablas de segundo nivel.
	for(i = 0; i< nro_tablas_segundo_nivel; i++){
		t_tabla_segundo_nivel *segundoNivel = list_get(tablas_segundo_nivel_list, *(primerNivel->tablas_asociadas+i));

		//Itero por las paginas de la tabla de segundo nivel y veo si esta en uno su bit de presencia.
		for(j=0; j< valores_generales_memoria->pagPorTabla; j++){
			t_paginas_en_tabla *pagina = segundoNivel->paginas+j;

			//Bit de presencia en uno => desalojo esa pagina del marco en el que esta.
			uint32_t comienzoDelMarco = pagina->marco * valores_generales_memoria->tamPagina;
			uint32_t finDelMarco = comienzoDelMarco + valores_generales_memoria->tamPagina;

			//Saco la pagina del marco y la mando al swap correspondiente.
			uint32_t inicioPagina = pagina->id_pagina * valores_generales_memoria->tamPagina;

			if(pagina->bit_presencia == 1 ){
				if(pagina->bit_modificado == 1){

				//Copio el marco en el swap.

				//copio la pagina reemplazada en el swap.
				lseek(fd, inicioPagina, SEEK_SET);
				usleep(valores_generales_memoria->retardoSwap * 1000);
				write(fd, memoria_principal+comienzoDelMarco, valores_generales_memoria->tamPagina);
				}

			//Saco la pagina del marco y dejo el marco en 0. /
			for(k = comienzoDelMarco; k < finDelMarco; k++){
				memset(memoria_principal+k, 0, 1);
			}
			bitarray_clean_bit(bitmap_memoria, pagina->marco);
			pagina->bit_presencia = 0;
			log_info(logger,"\nSe ha liberado el marco %d de memoria.", pagina->marco);

			}
		}
	}
	//ENVIAR MENSAJE A KERNEL CON PROCESO DESALOJADO
	uint32_t result = 1;
	uint32_t cod_op= SUSPENDED;
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	memcpy(stream,&cod_op,sizeof(uint32_t));
	memcpy(stream+ sizeof(uint32_t),&result,sizeof(uint32_t));
	send(socket,stream,tamanio,NULL);
	liberarPcb(unPcb);
	free(stream);
	close(fd);

	queue_clean(primerNivel->paginas_en_memoria);
	log_info(logger,"\nSe ha liberado el espacio del proceso %d de memoria", unPcb->id);
}

void *liberarProcesoDeMemoriaYDeleteSwap(uint32_t socket){
	//Ver si recibir pcb o tabla de paginas.
    unPcb = recibir_pcb(socket);
	log_info(logger,"\nProceso %d para liberar en memoria y eliminar swap\n", unPcb->id);
	uint32_t *fd;
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

	usleep(valores_generales_memoria->retardoSwap * 1000);
	remove(nombreArchivo);

	//Busco la tabla de primer nivel.
    t_tabla_primer_nivel *primerNivel;
	primerNivel = list_get(tablas_primer_nivel_list, unPcb->tablaDePaginas);

	//Itero por las tablas de segundo nivel.
	for(i = 0; i< nro_tablas_segundo_nivel; i++){
		t_tabla_segundo_nivel *segundoNivel;
		uint32_t id_segundo_nivel = *(primerNivel->tablas_asociadas+i);
		segundoNivel = list_get(tablas_segundo_nivel_list, id_segundo_nivel);

		//Itero por las paginas de la tabla de segundo nivel y veo si esta en uno su bit de presencia.
		for(j=0; j< valores_generales_memoria->pagPorTabla; j++){
			t_paginas_en_tabla *pagina;
			pagina = segundoNivel->paginas+j;

			//Bit de presencia en uno => desalojo esa pagina del marco en el que esta.
			if(pagina->bit_presencia == 1){

				uint32_t comienzoDelMarco = pagina->marco * valores_generales_memoria->tamPagina;
				uint32_t finDelMarco = comienzoDelMarco + valores_generales_memoria->tamPagina;

				//Saco la pagina del marco y la mando al swap correspondiente.
				uint32_t IncioPagina = pagina->id_pagina * valores_generales_memoria->tamPagina;
		
				for(k = comienzoDelMarco; k < finDelMarco; k++){
					memset(memoria_principal+k, 0, 1);
				}

				list_clean(primerNivel->paginas_en_memoria->elements);
				bitarray_clean_bit(bitmap_memoria, pagina->marco);
				pagina->bit_presencia = 0;

			}
		}
		free(segundoNivel->paginas);
		free(segundoNivel);
	}

	queue_destroy(primerNivel->paginas_en_memoria);
	free(primerNivel->tablas_asociadas);
	free(primerNivel);
	log_info(logger,"\nSe ha eliminado el proceso %d de memoria.\n", unPcb->id);

	uint32_t result = 1;
	uint32_t tamanio= sizeof(uint32_t)*2;
	void *stream= malloc(tamanio);
	uint32_t cod_op= DELETESWAP;
	memcpy(stream,&cod_op,sizeof(uint32_t));
	memcpy(stream+sizeof(uint32_t),&result,sizeof(uint32_t));
	send(socket,stream,tamanio,NULL);
	liberarPcb(unPcb);
	free(stream);

}

void *devolver_valor_memoria(uint32_t socket){

	uint32_t direccion_fisica;
	uint32_t valor_memoria;

	recv(socket, &direccion_fisica, sizeof(uint32_t), MSG_WAITALL);

	memcpy(&valor_memoria,memoria_principal+direccion_fisica,sizeof(uint32_t));

	log_info(logger,"El valor de memoria de la direccion %d es: %d\n", direccion_fisica, valor_memoria);

	log_info(logger,"Valor de memoria %d enviado a Cpu.\n", valor_memoria);	

	uint32_t valor_a_enviar = (uint32_t)valor_memoria;

	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&valor_a_enviar,sizeof(uint32_t));

	usleep(valores_generales_memoria->retardoMemoria * 1000);

	send(socket,stream,sizeof(uint32_t),NULL);
	free(stream);
	
}

void escribir_en_memoria(uint32_t socket){
	uint32_t direccion_fisica, valor_a_escribir;
	uint32_t id_tabla_1er_nivel, id_tabla_2do_nivel, entrada_tabla_2do_nivel;
	uint32_t resultadoOp;
	recv(socket, &direccion_fisica, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &valor_a_escribir, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &id_tabla_1er_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &id_tabla_2do_nivel, sizeof(uint32_t), MSG_WAITALL);
	recv(socket, &entrada_tabla_2do_nivel, sizeof(uint32_t), MSG_WAITALL);
	
	memcpy(memoria_principal+direccion_fisica, &valor_a_escribir,sizeof(uint32_t));
	uint32_t valorEscrito;
	memcpy(&valorEscrito, memoria_principal+direccion_fisica, sizeof(uint32_t));
	log_info(logger,"\nValor escrito: %d", valorEscrito);

	//actualiza la tabla de paginas

	t_tabla_primer_nivel *t1= list_get(tablas_primer_nivel_list,id_tabla_1er_nivel);
	uint32_t idT2 = *(t1->tablas_asociadas+id_tabla_2do_nivel);
	t_tabla_segundo_nivel *t2= list_get(tablas_segundo_nivel_list,idT2);
	(t2->paginas+entrada_tabla_2do_nivel)->bit_modificado = 1;

	resultadoOp = 1;
	usleep(valores_generales_memoria->retardoMemoria * 1000);
	send(socket,&resultadoOp,sizeof(uint32_t),NULL);
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
				devolver_valor_memoria(socket);
				break;
			case WRITE:
				escribir_en_memoria(socket);
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

