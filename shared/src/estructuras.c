#include "../../shared/include/estructuras.h"


void liberarPcb(pcb* pcbALiberar)
{
    
    void liberarListaDeInstrucciones(instr_t* instruccion)
    {
        free(instruccion->id);
        free(instruccion->param);
        free(instruccion);
    }
    list_destroy_and_destroy_elements(pcbALiberar->instr, liberarListaDeInstrucciones);
    free(pcbALiberar);
}
