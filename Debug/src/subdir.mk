################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ll_1.cpp 

OBJS += \
./src/ll_1.o 

CPP_DEPS += \
./src/ll_1.d 


# Each subdirectory must supply rules for building sources it contributes
src/ll_1.o: ../src/ll_1.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -D__cplusplus=201402L -O1 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/ll_1.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


