#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
t_log *logger;
uint32_t kernel_socket;
uint32_t socket_dispatch;
uint32_t socket_interrupt;
uint32_t socket_memoria;
uint32_t interrupcion;
//Estructura para poder escuchar y conectarnos a todos los puertos
typedef struct {
	char* ip;
	char* puerto;
} config_conex;

//Estructura para poder realizar PCB.
typedef struct {
	char* alg_planif;
	uint32_t est_inicial;
	double alfa;
	uint32_t grad_multiprog;
	uint32_t max_block;
} gralStruct;


t_queue* estadoNew;
t_queue* estadoReady;
t_queue* estadoBlock;
t_queue* estadoBlockSusp;
t_queue* estadoReadySusp;
t_queue* estadoExec;
t_queue* estadoExit;

uint32_t nro_proceso;

gralStruct *valores_generales;
config_conex* config_valores_kernel;
config_conex* config_valores_memoria;
config_conex* config_valores_cpu_dispatch;
config_conex* config_valores_cpu_interrupt;
//falta puerto escucha.

char* path_kernel_config;

void paquete_pcb(pcb *proceso, int conexion);
void load_configuration();
void paquete_uint(uint32_t* numero, uint32_t* conexion);
void calcularEstimacionPcbBloqueado(pcb* proceso);
void calcularEstimacionPcbDesalojado(pcb* proceso);
float time_diff_Mediano(struct timeval *start, struct timeval *end);

#endif /* SRC_UTILS_H_ */
