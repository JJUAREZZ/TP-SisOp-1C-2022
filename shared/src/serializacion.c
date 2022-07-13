#include "../include/serializacion.h"
#include "../../shared/include/estructuras.h"

void crear_buffer(t_paquete* paquete) //Le pasamos un paquete para que le inyecte un buffer
{
	paquete->buffer = malloc(sizeof(t_buffer)); //reserva memoria para la struct t_buffer (devuelve un puntero)
	paquete->buffer->size = 0;			//inicializa el tamanio del payload del buffer en 0
	paquete->buffer->stream = NULL;		//el payload apunta a NULL de momento
}



void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);
	char* ne= paquete->buffer->stream;

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}



t_paquete* crear_paquete(op_code codigo_de_operacion) //Nos crea un paquete con un buffer listo para usar.
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_de_operacion;
	crear_buffer(paquete); // Le inyecta un buffer
	return paquete;
}


void agregar_a_paquete(t_paquete* paquete, t_proceso *proceso){

	void calcularTamanio(instr_t *instruccion){
		paquete->buffer->size+= (sizeof(int) *2 )  //nroDeParam y tamanio del id
								+ (sizeof(int)* instruccion->nroDeParam) //parametros
								+instruccion->idLength +1;   //id						
	}
	paquete->buffer->size+= sizeof(uint32_t);
	list_iterate(proceso->instrucciones,calcularTamanio); //calculo el tamanio del buffer
	void* stream= malloc(paquete->buffer->size); //reservo memoria para el buffer
	int offset=0; //desplazamiento

	memcpy(stream +offset, &proceso->tamanio,sizeof(uint32_t));
	offset+= sizeof(uint32_t);
	void copiarElementos(instr_t *instruccion){
		memcpy(stream + offset, &instruccion->idLength, sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, instruccion->id, instruccion->idLength +1);
		offset += instruccion->idLength +1;
		memcpy(stream + offset, &instruccion->nroDeParam, sizeof(int));
		offset += sizeof(int);
		for(int i=0; i< instruccion->nroDeParam; i++){
			memcpy(stream + offset,(instruccion->param)+i, sizeof(int));
			offset+=sizeof(int);
		}
	}
	list_iterate(proceso->instrucciones,copiarElementos);
	paquete->buffer->stream= stream;
	//mem_hexdump(stream,paquete->buffer->size);

}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes); //se reserva espacio para el Stream. Se utiliza void* porque no sabemos que tipo de dato es
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int)); //Se copia primero el cod de operacion
	desplazamiento+= sizeof(int); 											// se aumenta el desplazamiento
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int)); //se copia el tamanio del buffer
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);//se copia el buffer
	desplazamiento+= paquete->buffer->size;

	return magic; //retorna el puntero al stream
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int); // almacena en bytes, el tamanio del paquete: tamanio del payload(size) + tamanio de size + tamanio de codigo_operacion
	void* a_enviar = serializar_paquete(paquete, bytes); //dado un paquete y su tamanio lo serializa. Devuelve un puntero a el STREAM que se enviara finalmente.

	send(socket_cliente, a_enviar, bytes, 0); //envia el STREAM

	free(a_enviar);			//Libera el puntero al STREAM
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

//funciones del servidor

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	return buffer;
}

t_proceso* recibir_paquete(int socket_cliente)
{
	int tamanio;
	int desplazamiento = 0;
	void *buffer = recibir_buffer(&tamanio, socket_cliente);
	t_proceso *proceso= malloc(sizeof(t_proceso));
	t_list* valores = list_create();
	
	
	memcpy(&proceso->tamanio, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	 while(desplazamiento < tamanio)
	{
		instr_t *instruccion= malloc(sizeof(instr_t));
		memcpy(&instruccion->idLength, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->id= malloc(instruccion->idLength +1);
		memcpy(instruccion->id, buffer + desplazamiento, instruccion->idLength +1);
		desplazamiento+= instruccion->idLength +1;
		memcpy(&instruccion->nroDeParam, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->param= malloc(sizeof(int)* instruccion->nroDeParam);
		for(int i=0; i< instruccion->nroDeParam; i++){
			memcpy((instruccion->param)+i, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
		}
	
		list_add(valores, instruccion);
	}
	proceso->instrucciones= valores;
	free(buffer);
	return proceso;
}

pcb * recibir_pcb(int socket)
{
	uint32_t tamanio;
	uint32_t desplazamiento=0;
	void* buffer=recibir_buffer(&tamanio,socket);
	pcb* pcb_proceso= malloc(sizeof(pcb));
	t_list *instrucciones= list_create();

	memcpy(&pcb_proceso->id, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&pcb_proceso->tamanioProceso,buffer+desplazamiento,sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&tamanio, buffer+desplazamiento,sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	
	for(int i=0; i<tamanio; i++)
	{
		instr_t *instruccion= malloc(sizeof(instr_t));
		memcpy(&instruccion->idLength, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->id= malloc(instruccion->idLength +1);
		memcpy(instruccion->id, buffer + desplazamiento, instruccion->idLength +1);
		desplazamiento+= instruccion->idLength +1;
		memcpy(&instruccion->nroDeParam, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		instruccion->param= malloc(sizeof(int)* instruccion->nroDeParam);
		for(int i=0; i< instruccion->nroDeParam; i++){
			memcpy((instruccion->param)+i, buffer + desplazamiento, sizeof(int));
			desplazamiento+=sizeof(int);
		}
		list_add(instrucciones, instruccion);
	}
	pcb_proceso->instr= instrucciones;

	memcpy(&pcb_proceso->programCounter, buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&pcb_proceso->tablaDePaginas,buffer+desplazamiento,sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&pcb_proceso->estimacion_rafaga_actual,buffer+desplazamiento,sizeof(float));
	desplazamiento += sizeof(float);
	memcpy(&pcb_proceso->estimacion_rafaga_anterior,buffer+desplazamiento,sizeof(float));
	desplazamiento += sizeof(float);
	memcpy(&pcb_proceso->cpu_anterior,buffer+desplazamiento,sizeof(float));
	desplazamiento += sizeof(float);

	free(buffer);
	return pcb_proceso;
}


//funciones de memoria

void agregarPcbAPaquete(t_paquete *paquete,pcb *pcb){
	paquete->buffer->size+= (sizeof(uint32_t) * 5) + (sizeof(float)*3);

	void calcularTamanioDeInstrucciones(instr_t *instruccion)
	{
		paquete->buffer->size+= (sizeof(int) *2 )  
								+ (sizeof(int)* instruccion->nroDeParam) 
								+instruccion->idLength +1;   
	}
	list_iterate(pcb->instr,calcularTamanioDeInstrucciones); 
	void* stream= malloc(paquete->buffer->size); 
	int offset=0; 

	memcpy(stream +offset, &pcb->id,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream +offset, &pcb->tamanioProceso,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	uint32_t tamanio_lista= list_size(pcb->instr);
	memcpy(stream +offset, &tamanio_lista,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	void copiarElementos(instr_t *instruccion){
		memcpy(stream + offset, &instruccion->idLength, sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, instruccion->id, instruccion->idLength +1);
		offset += instruccion->idLength +1;
		memcpy(stream + offset, &instruccion->nroDeParam, sizeof(int));
		offset += sizeof(int);
		for(int i=0; i< instruccion->nroDeParam; i++){
			memcpy(stream + offset,(instruccion->param)+i, sizeof(int));
			offset+=sizeof(int);
		}
	}
	list_iterate(pcb->instr,copiarElementos);

	memcpy(stream +offset, &pcb->programCounter,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream +offset, &pcb->tablaDePaginas,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream +offset, &pcb->estimacion_rafaga_actual,sizeof(float));
	offset += sizeof(float);
	memcpy(stream +offset, &pcb->estimacion_rafaga_anterior,sizeof(float));
	offset += sizeof(float);
	memcpy(stream +offset, &pcb->cpu_anterior,sizeof(float));
	offset += sizeof(float);
	
	paquete->buffer->stream= stream;
}
