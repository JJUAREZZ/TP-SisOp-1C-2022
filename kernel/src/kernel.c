#include "../include/kernel.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

int main(){

	//create_kernel_logger();
	load_configuration();
	kernel_server_init(); 
	

	return 0;
}
