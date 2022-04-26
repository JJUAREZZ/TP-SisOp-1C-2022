/*
 * kernel_logger.c
 *
 *  Created on: 25 abr. 2022
 *      Author: utnso
 */
#include "kernel_logger.h"

int kernel_logger_create(char* logfile_name)
{
	kernel_log = logger_create(logfile_name, "KERNEL");
	if (kernel_log == NULL || kernel_log < 0)
	{
		perror("No ha sido posible instanciar el kernel_logger");
		return -1;
	}

	logger_print_header(kernel_log, "KERNEL");

	return 1;
}

void kernel_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(kernel_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void kernel_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(kernel_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void kernel_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(kernel_log, formated_message);
	free(formated_message);
	va_end(arguments);
}
