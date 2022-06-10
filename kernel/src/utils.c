#include "../include/utils.h"

void load_configuration(){

	t_config* config = config_create("/home/utnso/workspace/tp-2022-1c-Messirve/kernel/cfg/kernel.config");


	if(config == NULL){
		perror("Archivo de configuracion no encontrado");
	}

	//Lleno los struct con los valores de IP y PUERTO de cada uno que necesitamos.
	config_valores_kernel = malloc(sizeof(config_conex));
	config_valores_kernel->ip = config_get_string_value(config, "IP_KERNEL");
	config_valores_kernel->puerto = config_get_string_value(config, "PUERTO_KERNEL");

	config_valores_memoria = malloc(sizeof(config_conex*));
	config_valores_memoria->ip = config_get_string_value(config, "IP_MEMORIA");
	config_valores_memoria->puerto = config_get_string_value(config, "PUERTO_MEMORIA");

	config_valores_cpu_dispatch = malloc(sizeof(config_conex*));
	config_valores_cpu_dispatch->ip =config_get_string_value(config, "PUERTO_CPU_DISPATCH");
	config_valores_cpu_dispatch->puerto = config_get_int_value(config, "PUERTO_CPU_DISPATCH");

	config_valores_cpu_interrupt = malloc(sizeof(config_conex*));
	config_valores_cpu_interrupt->ip = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
	config_valores_cpu_interrupt->puerto = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");

	//Lleno los struct de los campos que necesitamos para el pcb y demas.
	valores_generales = malloc(sizeof(gralStruct));
	valores_generales->alg_planif = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	valores_generales->est_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	valores_generales->alfa = config_get_double_value(config, "ALFA");
	valores_generales->grad_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	valores_generales->max_block = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");

	//kernel_logger_info("IP_KERNEL: %s", config_valores->ip_kernel);
	//kernel_logger_info("PUERTO_KERNEL: %d", config_valores->puerto_kernel);
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

int inicializar_semaforo(sem_t** semaphore, char * sem_name, uint32_t initial_value){
	// O_CREAT significa "Crear el semaforo si no existe, y si existe abrirlo"
	// S_IRWU significa "Si se crea el semaforo, hacerlo con permisos Read Write eXecute para el propietario del semaforo"
	sem_t * openned_semaphore = sem_open(sem_name, O_CREAT, S_IRWXU, initial_value);

	if (openned_semaphore == SEM_FAILED){
		perror("sem_open");
		return -1;
		}
	else {
		printf("Semaphore [%s] openned successfully\n", sem_name);
		*semaphore = openned_semaphore;
		return 0;
	}
}
