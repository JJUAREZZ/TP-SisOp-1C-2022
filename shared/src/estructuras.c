#include "../../shared/include/estructuras.h"


void liberarPcb(pcb* pcbALiberar)
{
    void liberarListaDeInstrucciones(instr_t* instruccion)
    {
        free(instruccion->id);
        free(instruccion->param);
        free(instruccion);
    }
    list_iterate(pcbALiberar->instr,liberarListaDeInstrucciones);
    free(pcbALiberar);
}
