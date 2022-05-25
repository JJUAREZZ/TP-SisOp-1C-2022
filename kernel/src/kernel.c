#include "../include/kernel.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

int main(){

	//Estado de planificador;
	t_list* estadoNew;
	t_list* estadoBlock;
	t_list* estadoBlockSusp;
	t_list* estadoReadySusp;
	t_list* estadoExec;
	t_list* estadoExit;
	t_list* estadoReady;
	
	estadoNew 	= list_create();
	estadoReady = list_create();
	estadoBlock = list_create();
	estadoBlockSusp = list_create();
	estadoReadySusp = list_create();
	estadoExec = list_create();
	estadoExit = list_create();

	//create_kernel_logger();
	load_configuration();
	kernel_server_init();
	

	return 0;
}
