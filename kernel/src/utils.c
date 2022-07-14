#include "../include/utils.h"

void load_configuration(){

	t_config* config = config_create(path_kernel_config);


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.
	config_valores_kernel = malloc(sizeof(config_conex));
	char* ip_kernel =  config_get_string_value(config, "IP_KERNEL");
	config_valores_kernel->ip = malloc(string_length(ip_kernel));
	memcpy(config_valores_kernel->ip, ip_kernel, string_length(ip_kernel)+1);
	char* puerto_kernel = config_get_string_value(config, "PUERTO_ESCUCHA");
	config_valores_kernel->puerto = malloc(string_length(puerto_kernel));
	memcpy(config_valores_kernel->puerto, puerto_kernel, string_length(puerto_kernel)+1);

	config_valores_memoria = malloc(sizeof(config_conex*));
	char* ip_mem = config_get_string_value(config, "IP_MEMORIA");
	config_valores_memoria->ip = malloc(string_length(ip_mem));
	memcpy(config_valores_memoria->ip, ip_mem, string_length(ip_mem)+1);
	char* puerto_mem = config_get_string_value(config, "PUERTO_MEMORIA");
	config_valores_memoria->puerto = malloc(string_length(puerto_mem));
	memcpy(config_valores_memoria->puerto, puerto_mem, string_length(puerto_mem)+1);

	config_valores_cpu_dispatch = malloc(sizeof(config_conex*));
	char* ip_cpu = config_get_string_value(config, "IP_CPU");
	config_valores_cpu_dispatch->ip = malloc(string_length(ip_cpu));
	memcpy(config_valores_cpu_dispatch->ip, ip_cpu, string_length(ip_cpu)+1);
	char* puerto_disp = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	config_valores_cpu_dispatch->puerto = malloc(string_length(puerto_disp));
	memcpy(config_valores_cpu_dispatch->puerto, puerto_disp, string_length(puerto_disp)+1);

	config_valores_cpu_interrupt = malloc(sizeof(config_conex*));
	config_valores_cpu_interrupt->ip = malloc(string_length(ip_cpu));
	memcpy(config_valores_cpu_interrupt->ip, ip_cpu, string_length(ip_cpu));
	char* puerto_int = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	config_valores_cpu_interrupt->puerto = malloc(string_length(puerto_int));
	memcpy(config_valores_cpu_interrupt->puerto, puerto_int, string_length(puerto_int)+1);

	//Lleno los struct de los campos que necesitamos para el pcb y demas.
	valores_generales = malloc(sizeof(gralStruct));
	char* alg_planif = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	valores_generales->alg_planif = malloc(string_length(alg_planif));
	memcpy(valores_generales->alg_planif, alg_planif, string_length(alg_planif)+1);
	valores_generales->est_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	valores_generales->alfa = config_get_double_value(config, "ALFA");
	valores_generales->grad_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	valores_generales->max_block = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");

	config_destroy(config);
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