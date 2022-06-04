#include "../include/memoria.h"
#include "../include/utils.h"
#include "../../shared/include/serializacion.h"


void *atenderConexion(uint32_t );
void *retornar_id_tabla_de_pagina(uint32_t );
uint32_t memoria_socket;

int main()
{
    load_configuration();

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
		uint32_t socket;
		if ((socket = accept(memoria_socket,(struct sockaddr *) &client_info, &addrlen)) != -1)
		{
			pthread_t atenderConexion;
			pthread_create(&atenderConexion,NULL,atenderConexion,socket);
			
		}
	}
    return 0;
}

void *atenderConexion(uint32_t socket)
{
	pthread_t retornar_id_tabla_de_pagina;
	uint32_t cod_op= recibir_operacion(socket);
	if(cod_op>0)
	{
		switch (cod_op)
		{
		case TABLADEPAGINA:
			pthread_create(&retornar_id_tabla_de_pagina, NULL, retornar_id_tabla_de_pagina, socket);
			break;
		default:
			;
		}
	}
}

void *retornar_id_tabla_de_pagina(uint32_t socket)
{
	pcb *unPcb= recibir_pcb(socket);
	uint32_t id = rand();  //LUEGO HAY QUE VER COMO ASIGNAMOS ESE ID. POR AHORA LE METI UN RAND FALOPA
	void *stream= malloc(sizeof(uint32_t));
	memcpy(stream,&id,sizeof(uint32_t));
	send(socket,stream,sizeof(uint32_t),0);
}
