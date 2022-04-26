/*
 * kernel_logger.h
 *
 *  Created on: 25 abr. 2022
 *      Author: utnso
 */

#ifndef SRC_LOGGER_KERNEL_LOGGER_H_
#define SRC_LOGGER_KERNEL_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>

int  kernel_logger_create(char* logfile_name);
void kernel_logger_info(char* message, ...);
void kernel_logger_warn(char* message, ...);
void kernel_logger_error(char* message, ...);

t_log* kernel_log;


#endif /* SRC_LOGGER_KERNEL_LOGGER_H_ */
