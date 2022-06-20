#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"


void *retornar_id_tabla_de_pagina(uint32_t);
void *atenderConexion(uint32_t );
uint32_t memoria_socket;
pcb *unPcb;

int main()
{
    load_configuration();
	crear_memoria_principal();

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
			pthread_create(&hilo, NULL, retornar_id_tabla_de_pagina, socket);
			break;
		default:
			;
		}
	}
	pthread_join(retornar_id_tabla_de_pagina,NULL);
}

void *retornar_id_tabla_de_pagina(uint32_t socket)
{
	unPcb= recibir_pcb(socket);
	int id= unPcb->id +5; //LUEGO HAY QUE VER COMO ASIGNAMOS ESE ID.
	printf("\nRecibi un proceso: nroDeProceso= %d", unPcb->id);
	printf("\nAsignando tabla de pagina: id= %d\n",id);
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&id,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),NULL);
}
