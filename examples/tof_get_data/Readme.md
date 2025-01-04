# TOF传感器获取数据示例
## 硬件介绍
**Tof传感器**是一种基于飞行时间的传感器，对应的扩展板名为SLAM-deck，上面搭载了VL53L5cx传感器，可以感知到8*8个区域的数据。
## 代码结构
要想驱动一个VL53L5cx传感器，代码结构分为三个部分：
1. 硬件抽象层（Hardware Abstract Layer）部分
该部分位于Athena-Driver/Athena_deck_HAL/src/i2c_drv.c中，负责在I2C总线级别驱动传感器，用户无需修改。
2. 板级支持包（Board Support Packet）部分
位于Athena-Driver/BSP/VL53L5CX_ULD_API中，里面的src/vl53l5cx_api.c文件提供了所有用户可用的API，对应API的介绍在VL53L5CX的user manual中，如果对VL53L5CX有更多要求的用户，一定要认真阅读该API对应的手册，再使用。否则，建议按照本示例中代码使用。
3. 初始化部分
SLAM-deck需要进行两次初始化，一是上面的I2C-expander的初始化，二是VL53L5CX传感器的初始化，必须完成这两个部分，才能采集数据点。
4. 采集部分
使用本示例中给出的函数，完成采集，采集的结果需要根据VL53L5的手册进行分析。
## 注意事项
1. 系统启动之后必须延迟5秒，必须确保I2C总线不被误触。因为I2C一旦被误触，则必须关机重启。后续会加上重置的函数。
2. SLAM-deck有好有坏，代码不能执行，首先怀疑SLAM-deck有问题。
3. 本示例是ETH-PBL版本的SLAM-deck示例，一两个月内，我们会推出NetSI版本的SLAM-deck，届时实例会有变化，请关注。
## 相关文档
https://www.st.com/en/imaging-and-photonics-solutions/vl53l5cx.html
中有所有的文档。
