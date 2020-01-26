################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/md_processor.cpp 

OBJS += \
./src/md_processor.o 

CPP_DEPS += \
./src/md_processor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ --std=c++17 -DLOG4CXX_PATH=\"resources/xml/udp_publisher.log4cxx.xml\" -I"simulation/md_processor/src" -I"simulation/md_processor/src/vectorclass" -O3 -Wall -c -fmessage-length=0 -mavx2 -mfma -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


