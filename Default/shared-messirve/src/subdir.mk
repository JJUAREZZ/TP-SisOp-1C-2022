################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shared-messirve/src/connect.c 

OBJS += \
./shared-messirve/src/connect.o 

C_DEPS += \
./shared-messirve/src/connect.d 


# Each subdirectory must supply rules for building sources it contributes
shared-messirve/src/%.o: ../shared-messirve/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


