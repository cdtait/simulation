################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/md.cpp 

OBJS += \
./src/md.o 

CPP_DEPS += \
./src/md.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/obod/workspace_kepler/market_data_processor/src" -I"/home/obod/workspace_kepler/market_data_processor/src/vectorclass" -O3 -Wall -c -fmessage-length=0 -std=c++11 -fabi-version=4 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


