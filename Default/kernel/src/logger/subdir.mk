################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../kernel/src/logger/kernel_logger.c 

OBJS += \
./kernel/src/logger/kernel_logger.o 

C_DEPS += \
./kernel/src/logger/kernel_logger.d 


# Each subdirectory must supply rules for building sources it contributes
kernel/src/logger/%.o: ../kernel/src/logger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


