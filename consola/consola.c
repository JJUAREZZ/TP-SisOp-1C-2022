#include "utils.h"

int main(int argc, char** argv){

	int tamanio_proceso= atoi(argv[1]);
	char* path_instrucciones= (char*) argv[2];
	char ** lista;

	printf("tamanio en bytes recibido: %d \n", tamanio_proceso);
	printf("path recibido: %s", path_instrucciones);
	printf("\n");

	crear_lista_de_instrucciones(path_instrucciones,&lista);


	for(int i=0 ; i<5; i++)
		printf("\n%s", lista[i]);

	printf("\n");

	 return 0;

}
