#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include <commons/collections/list.h>
#include "../../shared/include/estructuras.h"

typedef enum
{
	MENSAJE,
	PAQUETE,
	PCB,
	TABLADEPAGINA,
	BLOCKED,
	TERMINATED,
	PROCESOTERMINATED,
	PROCESODESALOJADO,
	DESALOJARPROCESO,
	READ,
	WRITE,
	COPY,
	CPUVACIA,
	HANDSHAKE_CPU,
	IDTABLASEGUNDONIVEL,
	MARCO,
	SUSPENDED,
	DELETESWAP
}op_code;

typedef struct
{
	int size;   	//tamanio payload
	void* stream;	//payload
} t_buffer;

//Esta es una estructura intermedia que nos permite almacenar lo que vamos a enviar como paquete
//size contiene el tamanio del payload (los datos que vamos a enviar) y
//stream es un puntero al HEAP(lugar donde esta el payload), el cual contiene los datos en cuestion

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	uint32_t tamanio;
	t_list *instrucciones;
}t_proceso;

//Esta estructura contiene un codigo de operacion -contiene info sobre lo que vamos a enviar- y al buffer.

void crear_buffer(t_paquete* );
t_paquete* crear_paquete(op_code);
void agregar_a_paquete(t_paquete*, t_proceso *);
void* serializar_paquete(t_paquete* , int );
void enviar_paquete(t_paquete* , int );
void eliminar_paquete(t_paquete* );
void enviar_mensaje(char* , int );
void agregarPcbAPaquete(t_paquete *,pcb *);

void* recibir_buffer(int*, int);
t_proceso* recibir_paquete(int);
pcb * recibir_pcb(int );
int recibir_operacion(int);
#endif
