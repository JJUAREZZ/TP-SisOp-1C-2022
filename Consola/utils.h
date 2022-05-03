#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>

typedef struct {
	char* id;
	t_list *param;
} instr_t;

t_list * crear_lista_de_instrucciones(char *path) {
	FILE* file = fopen(path, "r");
	struct stat stat_file;
	stat(path, &stat_file);

	if (file == NULL) {
		return NULL;
	}

	char* buffer = calloc(1, stat_file.st_size + 1);
	fread(buffer, stat_file.st_size, 1, file);

	for (int i = 1; !feof(file); i++) {
		fread(buffer, stat_file.st_size, 1, file);
	}

	char **inst_per_line = string_split(buffer, "\n");

	t_list *inst_list = list_create();

	void closure(char *line) {
		char** inst_and_param = string_split(line, " ");
		instr_t *single_inst = malloc(sizeof(instr_t));
		single_inst->param = list_create();
		single_inst->id = inst_and_param[0];
		for (int i = 1; inst_and_param[i] != NULL; i++) {
			list_add(single_inst->param, inst_and_param[i]);
		}

		list_add(inst_list, single_inst);
	}

	string_iterate_lines(inst_per_line, closure);

	free(buffer);
	free(inst_per_line);
	fclose(file);

	return inst_list;

}

typedef struct {
	char* ip_kernel;
	char* puerto_kernel;
} kernel_config;

kernel_config config_valores;

void cargar_configuracion(void){
	t_config* config = config_create("../consola.config");

	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	config_valores.ip_kernel = 		config_get_string_value(config, "IP_KERNEL");
	config_valores.puerto_kernel = 	config_get_string_value(config, "PUERTO_KERNEL");
}



#endif /* UTIL_H_ */