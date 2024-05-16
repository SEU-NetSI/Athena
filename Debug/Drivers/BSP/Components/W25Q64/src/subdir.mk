################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/W25Q64/src/w25q64.c 

OBJS += \
./Drivers/BSP/Components/W25Q64/src/w25q64.o 

C_DEPS += \
./Drivers/BSP/Components/W25Q64/src/w25q64.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/W25Q64/src/%.o Drivers/BSP/Components/W25Q64/src/%.su Drivers/BSP/Components/W25Q64/src/%.cyclo: ../Drivers/BSP/Components/W25Q64/src/%.c Drivers/BSP/Components/W25Q64/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L431xx -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"F:/athena-firmware/Drivers/Athena_deck_HAL/inc" -I"F:/athena-firmware/Drivers/BSP/Components/TCA6408A/inc" -I"F:/athena-firmware/Drivers/BSP/Components/VL53L5CX_ULD_API/inc" -I"F:/athena-firmware/app-example/tinymap/inc" -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I"F:/athena-firmware/Drivers/BSP/Components/W25Q64/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-W25Q64-2f-src

clean-Drivers-2f-BSP-2f-Components-2f-W25Q64-2f-src:
	-$(RM) ./Drivers/BSP/Components/W25Q64/src/w25q64.cyclo ./Drivers/BSP/Components/W25Q64/src/w25q64.d ./Drivers/BSP/Components/W25Q64/src/w25q64.o ./Drivers/BSP/Components/W25Q64/src/w25q64.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-W25Q64-2f-src

