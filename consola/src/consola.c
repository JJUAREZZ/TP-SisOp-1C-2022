#include "../../shared/include/sockets.h"
#include "../include/utils.h"

int main(int argc, char** argv) {

	int tamanio_proceso = atoi(argv[1]);
	char* path_instrucciones = (char*) argv[2];
	int conexion;

	printf("\nTamanio en bytes recibido: %d \n", tamanio_proceso);
	printf("Path recibido: %s", path_instrucciones);
	printf("\n");

	t_list *inst_list = crear_lista_de_instrucciones(path_instrucciones);

	//Muestro los Id de las instrucciones
	void closure(instr_t* element){
		printf("\n%s",element->id);
		for(int i=0; i<element->nroDeParam;i++){
			printf(" %d",(int) element->param[i]);
		}
	}
	list_iterate(inst_list,closure);
	
	cargar_configuracion();
	conexion= socket_connect_to_server(config_valores.ip_kernel, config_valores.puerto_kernel);
	//paquete(conexion);



}