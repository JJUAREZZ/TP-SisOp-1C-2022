#ifndef SRC_PLANIFICADORES_H_
#define SRC_PLANIFICADORES_H_

#include "utils.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

void *planificadorCorto(t_list*, t_log*);
void calcularEstimacionPcb(pcb*);
bool estimacionMayor (pcb*, pcb*);
void planificadorSrt(t_list* , t_log*);
void planificadorFifo(t_list* listaReady, t_log* unLogger);


//planificador a largo plazo

void planificadorALargoPlazo();
pcb *crearPcb(t_proceso *);
t_proceso* recibir_proceso(uint32_t
void *atenderProceso(uint32_t );
#endif /* SRC_PLANIFICADORES_H_ */