#ifndef SRC_PLANIFICADORES_H_
#define SRC_PLANIFICADORES_H_

#include "utils.h"
#include "../../shared/include/sockets.h"
#include "../../shared/include/serializacion.h"
#include "../../shared/include/estructuras.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <semaphore.h>

//semaforos
pthread_mutex_t COLANEW;
pthread_mutex_t COLAREADY;
pthread_mutex_t COLAEXEC;
pthread_mutex_t COLABLOCK;
pthread_mutex_t COLASUSPREADY;
pthread_mutex_t COLASUSPBLOCKED;
pthread_mutex_t COLAEXIT;
pthread_mutex_t NRODEPROCESO;
pthread_mutex_t PROCDESALOJADO;

pcb* pcbDesalojado; //aca se almacena el proceso devuelto por cpu

//Planificador a corto plazo

void *planificadorACortoPlazo();
void calcularEstimacionPcb(pcb*);
void planificadorSrt();
void planificadorFifo();

//Planificador a mediano plazo.
uint32_t resultado_suspension;
void *planificadorAMedianoPlazo();
void *bloquearProcesos();
void *enviarProcesosDeSuspendedReadyAReady();

//planificador a largo plazo

uint32_t id_tabla_pagina;
uint32_t resultado_delete_swap;
void planificadorALargoPlazo();
pcb *crearPcb(t_proceso *);
t_proceso* recibir_proceso(uint32_t);
void *atenderProceso(uint32_t );
uint32_t obtenerTablaDePagina(pcb *);
void enviarProcesosAReady();
void terminarProcesos();
#endif /* SRC_PLANIFICADORES_H_ */