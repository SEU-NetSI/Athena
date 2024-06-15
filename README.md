# Athena v3.2 efficiency Firmware
## Software Update Log
### v0.1
1. I2C transmit with DMA **@Hanjie**
2. slam-deck & I2C-expander BSP **@Hanjie**
3. FreeRTOS **@Hanjie**
### v0.1.1
Increased throughput of I2C using fast I/O speed & dual DMA buffer **@Hanjie**
### v0.2
1. slam-deck driver **@Hanjie**
2. SPI exchange with DMA **@Hanjie @Yunfan**
3. W25Q64 BSP **@Yunfan @Hanjie**
### v0.3
1. USART Tranmit with DMA LL **@Hao Li @Hanjie**
2. USART Receive with Interrupt+callback+semphr **@Hanjie @Hao Li**
## Hardware Update Log
### v3.2
STM32L496 + W25Q512 @8MHz
## How to use
git clone
## TBD
1. I2C semphr
2. FreeRTOS multi task
3. SPI speed up
4. printf reconstruct
## Contributors
@Hanjie Yu

@Yunfan Li

@Hao Li

**National Natural Science Foundation of China (6509009672)**


