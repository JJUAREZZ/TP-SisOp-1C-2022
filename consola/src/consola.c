#include "../include/utils.h"

int main(int argc, char** argv) {

	path_consola_config = (char*) argv[1];
	int tamanio_proceso = atoi(argv[2]);
	char* path_instrucciones = (char*) argv[3];
	int conexion;

	printf("\nTamanio en bytes recibido: %d \n", tamanio_proceso);
	printf("Path recibido: %s", path_instrucciones);
	printf("\n");

	t_proceso *proceso= malloc(sizeof(t_proceso));
	proceso->tamanio= tamanio_proceso;
	proceso->instrucciones= crear_lista_de_instrucciones(path_instrucciones);
	
//TODO hacer free de los punteros

	//Muestro los Id de las instrucciones
	void closure(instr_t* element)
	{
	printf("\n%s ",element->id);
		for(int i=0; i<element->nroDeParam;i++){
			printf(" %d",(int) element->param[i]);
		}
	}
	list_iterate(proceso->instrucciones,closure);
	
	cargar_configuracion();
	conexion= socket_connect_to_server(config_valores->ip_kernel, config_valores->puerto_kernel);
	paquete(proceso,conexion);
}

void paquete(t_proceso *proceso, int conexion){
	t_paquete *paquete= crear_paquete(PAQUETE);
	agregar_a_paquete(paquete,proceso);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}


