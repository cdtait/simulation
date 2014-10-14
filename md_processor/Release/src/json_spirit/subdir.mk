################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/json_spirit/json_spirit_reader.cpp \
../src/json_spirit/json_spirit_value.cpp \
../src/json_spirit/json_spirit_writer.cpp 

OBJS += \
./src/json_spirit/json_spirit_reader.o \
./src/json_spirit/json_spirit_value.o \
./src/json_spirit/json_spirit_writer.o 

CPP_DEPS += \
./src/json_spirit/json_spirit_reader.d \
./src/json_spirit/json_spirit_value.d \
./src/json_spirit/json_spirit_writer.d 


# Each subdirectory must supply rules for building sources it contributes
src/json_spirit/%.o: ../src/json_spirit/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/obod/workspace_kepler/market_data_processor/src" -I"/home/obod/workspace_kepler/market_data_processor/src/vectorclass" -O3 -Wall -c -fmessage-length=0 -std=c++11 -fabi-version=4 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


