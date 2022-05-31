#include "../include/planificadores.h"

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


//*****************************planificador a largo plazo******************************

void *atenderProceso(uint32_t accepted_fd)
{
	t_proceso *nuevoProceso= recibir_proceso(accepted_fd);
	pcb* nuevoPcb= crearPcb(nuevoProceso);
	//mutex
	queue_push(estadoNew,nuevoPcb);
	//fin de mutex
}

t_proceso* recibir_proceso(uint32_t accepted_fd){
	t_proceso* proceso= malloc(sizeof(t_proceso));
	uint32_t cod_op= recibir_operacion(accepted_fd);
	if(cod_op>0){
		switch (cod_op)
		{
		case PAQUETE:
			proceso = recibir_paquete(accepted_fd);
			log_info(logger,"Me llego el siguiente proceso:\n");
			printf("Tamanio del Proceso en bytes: %d", proceso->tamanio);
			printf("\nInstrucciones : \n");
			void mostrarInstrucciones(instr_t* element)
			{
				printf("%s ",element->id);
				for(int i=0; i<element->nroDeParam;i++)
					printf(" %d",(int) element->param[i]);
				printf("\n");
			}
			list_iterate(proceso->instrucciones, mostrarInstrucciones);
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	return proceso;
}		

pcb *crearPcb(t_proceso *proceso)
{
	pcb *pcbDelProceso= malloc(sizeof(pcb));
	//mutex para nro_proceso
	pcbDelProceso->id = nro_proceso;
	nro_proceso ++;
	//fin mutex
	pcbDelProceso->tamanioProceso= proceso->tamanio;
	pcbDelProceso->instr= proceso->instrucciones;
	pcbProceso->programCounter = 0;
	pcbDelProceso->tablaDePaginas = 0;
	pcbDelProceso->estimacion_rafaga_actual = valores_generales->est_inicial;
	free(proceso);
	printf("\nPCB del proceso creado");
	return pcbDelProceso;
}


void planificadorALargoPlazo()
 {
	 for(;;)
	 {

		 //falta checkear pcbs a finalizar

		 //mutex de las colas: ready, exec y blocked
		 uint32_t gradoDeMultiProgActual= queue_size(estadoBlock)+ queue_size(estadoReady)+
		 queue_size(estadoExec);
		 if(gradoDeMultiProgActual < valores_generales->grad_multiprog)
		 {
			 //enviar mensaje a memoria para obtener el valor de tabla de pagina
			 //wait memoria
			 //pcb *procesoAReady= queue_pop(estadoNew);
			 //procesoAReady->tablaDePaginas= lo que mande memoria
			 queue_push(estadoReady,queue_pop(estadoNew));
		 }
		 //fin del mutex
	 }
 }