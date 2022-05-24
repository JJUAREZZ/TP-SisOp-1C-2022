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

typedef enum
{
	MENSAJE,
	PAQUETE
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

//Esta estructura contiene un codigo de operacion -contiene info sobre lo que vamos a enviar- y al buffer.

void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, t_list *lista);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void enviar_mensaje(char* mensaje, int socket_cliente);

void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
int recibir_operacion(int);
#endif
