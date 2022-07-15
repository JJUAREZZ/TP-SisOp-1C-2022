#include "../include/utils.h"

void load_configuration(){

	t_config* config = config_create(path_kernel_config);


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.
	config_valores_kernel = malloc(sizeof(config_conex));
	config_valores_kernel->ip=config_get_string_value(config, "IP_KERNEL");
	config_valores_kernel->puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
	config_valores_memoria = malloc(sizeof(config_conex*));
	config_valores_memoria->ip= config_get_string_value(config, "IP_MEMORIA");
	config_valores_memoria->puerto = config_get_string_value(config, "PUERTO_MEMORIA");
	config_valores_cpu_dispatch = malloc(sizeof(config_conex*));
	config_valores_cpu_dispatch->ip= config_get_string_value(config, "IP_CPU");
	config_valores_cpu_dispatch->puerto= config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	config_valores_cpu_interrupt = malloc(sizeof(config_conex*));
	config_valores_cpu_interrupt->ip = config_get_string_value(config, "IP_CPU");
	config_valores_cpu_interrupt->puerto = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

	//Lleno los struct de los campos que necesitamos para el pcb y demas.
	valores_generales = malloc(sizeof(gralStruct));
	valores_generales->alg_planif= config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	valores_generales->est_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	valores_generales->alfa = config_get_double_value(config, "ALFA");
	valores_generales->grad_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	valores_generales->max_block = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");

	//config_destroy(config);
}

void paquete_pcb(pcb *proceso, int conexion){
	t_paquete *paquete= crear_paquete(PCB);
	agregarPcbAPaquete(paquete,proceso);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void paquete_uint(uint32_t* numero, uint32_t* conexion){
	t_paquete *paquete= crear_paquete(PAQUETE);
	agregarPcbAPaquete(paquete,numero);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void calcularEstimacionPcbBloqueado(pcb* proceso){
	proceso->estimacion_rafaga_anterior = proceso->estimacion_rafaga_actual;

	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior  * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa);
}

void calcularEstimacionPcbDesalojado(pcb* proceso){
	proceso->estimacion_rafaga_anterior = proceso->estimacion_rafaga_actual;

	proceso->estimacion_rafaga_actual = proceso->estimacion_rafaga_anterior - proceso->cpu_anterior;

}

float time_diff_Mediano(struct timeval *start, struct timeval *end)
{
    float seconds  = end->tv_sec  - start->tv_sec;
    float useconds = end->tv_usec - start->tv_usec;

    float tiempo_dif = seconds * 1000.0;
	tiempo_dif+= useconds/1000.0;
   // float tiempo_dif = (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
    return tiempo_dif;
}