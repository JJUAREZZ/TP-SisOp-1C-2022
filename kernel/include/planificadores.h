#ifndef SRC_PLANIFICADORES_H_
#define SRC_PLANIFICADORES_H_

#include "utils.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

void *planificadorCorto(t_list* listaReady, t_log* unLogger){
	
	int utilizarFifo = strcmp(valores_generales->alg_planif, "FIFO");
	int utilizarSrt = strcmp(valores_generales->alg_planif, "SRT");

	if(utilizarFifo == 0){
			//Ejecutar FIFO.
			//Esta linea no va a ser falta
			log_info(unLogger, "Planificador por FIFO.");
			planificadorFifo(listaReady, unLogger);
	}	
	if(utilizarSrt == 0){
			//hilo que ejecuta SRT.
			log_info(unLogger, "Planificador por SRT.");
			planificadorSrt(listaReady, unLogger);
	}    
}

void calcularEstimacionPcb(pcb* proceso){
	proceso->estimacion_rafaga_actual = 
		proceso->estimacion_rafaga_anterior * (1 - valores_generales->alfa) + proceso->cpu_anterior * (1 - valores_generales->alfa); 
}

//Ver si funciona.
bool estimacionMayor (pcb* proceso1, pcb* proceso2){
	if(proceso1->estimacion_rafaga_actual > proceso2->estimacion_rafaga_actual){
		true;
	}else{
		false;
	}
}

void planificadorSrt(t_list* listaReady, t_log* unLogger){
	int tamanioReady = list_size(listaReady);

	while(tamanioReady > 0){

	list_iterate(listaReady, calcularEstimacionPcb);
	list_sort(listaReady, estimacionMayor);

	}	
}

//ver si tiene que agarrar un proceso o una lista de ready.
void planificadorFifo(t_list* listaReady, t_log* unLogger){

	int tamanioReady = list_size(listaReady);

	while(tamanioReady > 0){

	pcb* elemEjecutar = list_remove(listaReady, 0);
	//Enviar Primer elemento de la lista a Cpu Dispatch
	int conexion_cpu_dispatch = socket_connect_to_server(config_valores_cpu_dispatch->ip, config_valores_cpu_dispatch->puerto);
	paquete_pcb(elemEjecutar, conexion_cpu_dispatch);
	log_info(unLogger, "Proceso enviado a CPU");

	//Recibir de blocked los Procesos.
	pcb* elemRecib; 

	}
}

#endif /* SRC_PLANIFICADORES_H_ */