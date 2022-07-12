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
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"

char* path_consola_config;

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


	char **inst_per_line = string_split(buffer, "\n"); //divido el archivo leido por lineas
	int tamanioArray(char ** array){
		int n=0;
		for(int i=0 ;*(array+i)!= NULL; i++)
			n++;
		return n;
	}

	t_list *inst_list = list_create();                 //creo la lista de instrucciones
	
	void closure(char *line) {
	    char** inst_and_param = string_split(line, " ");    //divido la linea por " "
		instr_t *single_inst = malloc(sizeof(instr_t));     //reservo memoria para estructura instr_t
	   single_inst->idLength= strlen(inst_and_param[0]);
	   single_inst->id= malloc(single_inst->idLength +1);
	    memcpy(&single_inst->id,&inst_and_param[0],single_inst->idLength +1);        //copio el id de la instruccion
	    single_inst->nroDeParam= tamanioArray(inst_and_param)-1;     //guardo la cantidad de parametros 
	    single_inst->param= malloc(sizeof(uint32_t)*single_inst->nroDeParam);  //reservo mem para los parametros
		for (int i = 1; i<=single_inst->nroDeParam; i++) {   //recorro el array a partir del id de la instruccion
			single_inst->param[i-1]= (uint32_t) atoi(inst_and_param[i]);	//copio los parametros
		}
		if(strcmp(single_inst->id, "NO_OP") == 0){
			uint32_t nroDeParam= single_inst->param[0];
			single_inst->nroDeParam= 0;
			free(single_inst->param);
			single_inst->param= NULL;
			for(int i=0;i<nroDeParam-1; i++){
				list_add(inst_list, single_inst);
			}
		}

		list_add(inst_list, single_inst);                   //agrego la instruccion a la lista
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

kernel_config *config_valores;

void cargar_configuracion(void){
	t_config* config = config_create(path_consola_config);
	config_valores= malloc(sizeof(kernel_config));

	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	config_valores->ip_kernel = config_get_string_value(config, "IP_KERNEL");
	config_valores->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
}



#endif /* UTIL_H_ */
