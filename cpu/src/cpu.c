#include "../include/cpu.h"
#include "../include/utils.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>



int main() {

    load_configuration_cpu();
    conectar_dispatcher();

    return 0;    
}

void *atenderPcb(uint32_t accepted_fd){
	pcb *nuevoPcb = recibir_pcb(accepted_fd);
	fetch(nuevoPcb);
}

pcb* recibir_pcb(uint32_t accepted_fd) {
    pcb* pcb= malloc(sizeof(pcb));
	uint32_t cod_op= recibir_operacion(accepted_fd);
	if(cod_op>0){
		switch (cod_op)
		{
		case PCB:
			pcb = recibir_paquete(accepted_fd);
			printf("Me llego el siguiente pcb:\n");
			printf("Tamanio del Proceso en bytes: %d", pcb->tamanioProceso);
			printf("\nInstrucciones : \n");
			/*void mostrarInstrucciones(instr_t* element)
			{
				printf("%s ",element->id);
				for(int i=0; i<element->nroDeParam;i++)
					printf(" %d",(int) element->param[i]);
				printf("\n");
			}
			list_iterate(pcb->instr, mostrarInstrucciones);*/
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	return pcb;
}

void fetch(pcb* pcb) {
	list_get(pcb->instr, pcb->programCounter);
}

//*** UTILIZAR sem_post(&semEnviarDispatch); CUANDO LA CPU ESTE DESOCUPADA ***
//*** CUANDO interrumpirCPU = 1, interrumpir la CPU y enviar el PCB a KERNEL *** 