################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app-example/tinymap/src/bresenham.c \
../app-example/tinymap/src/calibration.c 

OBJS += \
./app-example/tinymap/src/bresenham.o \
./app-example/tinymap/src/calibration.o 

C_DEPS += \
./app-example/tinymap/src/bresenham.d \
./app-example/tinymap/src/calibration.d 


# Each subdirectory must supply rules for building sources it contributes
app-example/tinymap/src/%.o app-example/tinymap/src/%.su app-example/tinymap/src/%.cyclo: ../app-example/tinymap/src/%.c app-example/tinymap/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L431xx -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"F:/athena-firmware/Drivers/Athena_deck_HAL/inc" -I"F:/athena-firmware/Drivers/BSP/Components/TCA6408A/inc" -I"F:/athena-firmware/Drivers/BSP/Components/VL53L5CX_ULD_API/inc" -I"F:/athena-firmware/app-example/tinymap/inc" -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I"F:/athena-firmware/Drivers/BSP/Components/W25Q64/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-app-2d-example-2f-tinymap-2f-src

clean-app-2d-example-2f-tinymap-2f-src:
	-$(RM) ./app-example/tinymap/src/bresenham.cyclo ./app-example/tinymap/src/bresenham.d ./app-example/tinymap/src/bresenham.o ./app-example/tinymap/src/bresenham.su ./app-example/tinymap/src/calibration.cyclo ./app-example/tinymap/src/calibration.d ./app-example/tinymap/src/calibration.o ./app-example/tinymap/src/calibration.su

.PHONY: clean-app-2d-example-2f-tinymap-2f-src

