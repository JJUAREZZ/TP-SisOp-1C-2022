#include "../include/kernel.h"
#include "../include/utils.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

int main(int argc, char** argv) {
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	path_kernel_config = (char*) argv[1];
	load_configuration();
	kernel_server_init(); 
	return 0;
}
