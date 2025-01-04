# 用户初始化示例
## 现有方法
在Athena上开发，每位用户创建任务的时候，都需要在Freertos.c中进行修改，例如：
```c
osThreadId_t TOF_exampleHandle;
const osThreadAttr_t tof_get_data_attributes = {
		.name = "tof_get_data",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
tof_get_data(void *para){
    while(1){
        fetch();
    }
}
```
是标准的做法，这种方法的缺点是：每位用户创建新任务后，都需要来回切换到Freertos.c文件中进行修改，这样不仅繁琐，而且如果对代码不熟悉，很容易出错。
## 改进方法
为了解决这个问题，我们提供了一个更加简单的方法，用户只需要在自己的代码中使用一个宏定义，就可以完成任务的创建，不需要再去Freertos.c文件中进行修改。这是通过将所有应用宏的函数编译到一个指定的段来实现的，系统初始化后，会自动扫描这个段，逐一执行用户的初始化函数。
## 使用方法
如init_fun.c所示，我们建议用户将自己的初始化函数写在一个函数中，目前暂时只支持void类型，例如：
```c
void user_init_fun(void){
    ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
    SemaphoreHandle_t txComplete = NULL;
    SemaphoreHandle_t rxComplete = NULL;
    SemaphoreHandle_t spiMutex = NULL;
    SemaphoreHandle_t UartRxReady = NULL;
    QueueHandle_t UartRxQueue = NULL;
    int a = 0;
    int b = 0;
    TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
    //任意代码都可以写在这里，但如果需要创建任务，请不要忘记加入任务控制块信息，控制块信息也加在用户自定义文件中    
}
```
接下来，只需要在自己的代码中加入一个结构体和宏定义。
```c
static const UserInit led_init = {
		.init = user_ledtask_init,
};
USER_INIT(led_init);
```
其中，UserInit结构体是我们定义的结构体，包含一个函数指针，这就是你需要指向用户自定义init的地方，当然，结构体还支持更多功能，用户可能会问，将所有初始化函数都写在一个入口，耦合性是否会很强。这个问题我们已经考虑到，UserInit中可以添加函数指针，如.test,.get,.set等等等等。

定义完之后，只需要将用户定义的结构体套用进USER_INIT宏定义中，就可以完成任务的创建，不需要再去Freertos.c文件中进行修改。