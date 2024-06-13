################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32l496rgtx.s 

OBJS += \
./Core/Startup/startup_stm32l496rgtx.o 

S_DEPS += \
./Core/Startup/startup_stm32l496rgtx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -c -I"F:/Athena-efficiency/athena-efficiency/Athena-Driver" -I"F:/Athena-efficiency/athena-efficiency/Drivers/CMSIS" -I"F:/Athena-efficiency/athena-efficiency/Drivers/STM32L4xx_HAL_Driver" -I"F:/Athena-efficiency/athena-efficiency/Middlewares/FreeRTOS" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32l496rgtx.d ./Core/Startup/startup_stm32l496rgtx.o

.PHONY: clean-Core-2f-Startup

