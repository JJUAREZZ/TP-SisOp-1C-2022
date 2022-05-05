#include "kernel.h"

int main(){

	create_kernel_logger();
	load_configuration();

	kernel_server_init();
	return 0;
}
