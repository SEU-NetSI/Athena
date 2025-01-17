# Low Power Driver 使用说明
## API介绍
1. **WKUP_Init**：初始化Wakeup引脚，配置Standby模式下调试相关寄存器
2. **Enter_Standby**：进入Standby模式
3. **Post_Standby**：退出Standby模式后，清理标志位
4. **ENABLE_DBG_STANDBY**宏：设置为0禁止H7在Standby模式下调试，设置为1允许H7在Standby模式下调试

## 示例程序
参考**Post_Standby**函数
```c
LL_mDelay(2000);
Post_Standby();
WKUP_Init();
Enter_Standby();
```
使用时要注意从Standby模式唤醒后，程序复位

## 注意事项
1. **不要让MCU一直处于Standby模式中**，例如
```c
void StartDefaultTask(void *argument)
{
  while(1)
  {
    Enter_Standby();
  }
}
```
MCU在进入Standby模式后会关闭几乎所有外设，也无法烧录进程序。让MCU一直处于Standby模式会导致MCU变砖。

**建议操作**：在Enter_Standby()前添加延时并加入周期性唤醒源。

2. 允许H7在Standby模式下调试会大幅度提升整板功耗，**默认禁止H7在Standby模式下调试**。
