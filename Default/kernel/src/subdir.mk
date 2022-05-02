################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../kernel/src/kernel.c 

OBJS += \
./kernel/src/kernel.o 

C_DEPS += \
./kernel/src/kernel.d 


# Each subdirectory must supply rules for building sources it contributes
kernel/src/%.o: ../kernel/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


