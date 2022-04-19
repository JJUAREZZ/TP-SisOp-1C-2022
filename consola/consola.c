#include "utils.h"

int main(int argc, char** argv) {

	int tamanio_proceso = atoi(argv[1]);
	char* path_instrucciones = (char*) argv[2];

	printf("\nTamanio en bytes recibido: %d \n", tamanio_proceso);
	printf("Path recibido: %s", path_instrucciones);
	printf("\n");

	t_list *inst_list = crear_lista_de_instrucciones(path_instrucciones);

	//Muestro los Id de las instrucciones
	for (int i = 0; i < (inst_list->elements_count); i++) {
		instr_t *instruction = list_get(inst_list, i);
		printf("\n%s", instruction->id);

	}

	return 0;

}
