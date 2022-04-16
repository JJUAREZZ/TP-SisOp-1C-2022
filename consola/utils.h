#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <commons/log.h>
#include<commons/string.h>



typedef struct{
	char* id;
	unsigned int numero_de_parametros;
	unsigned int *parametros;
}instr_t;

void crear_lista_de_instrucciones(char *path, char ***list) {
	FILE* file = fopen(path, "r");
	struct stat stat_file;
		stat(path, &stat_file);

	if (file == NULL) {
		return NULL;
	}

	char* buffer= calloc(1, stat_file.st_size + 1);
	fread(buffer, stat_file.st_size, 1, file);

	for(int i=1;!feof(file);i++){
		fread(buffer, stat_file.st_size, 1, file);
	}

	*list= string_split(buffer,"\n");

	free(buffer);
	fclose(file);

	return NULL;

}

