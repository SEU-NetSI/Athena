# DebugPrint输出示例
## 串口调试介绍（建议使用）
想要调用这段代码，需要借助一个usb转uart接口，分为以下五个步骤
 1.  硬件方面：需购买uart转usb转接板FT232
 2.  驱动方面：下载 USB转串口WINDOWS驱动程序
https://www.wch.cn/download/CH341SER_EXE.html
 3. PC端显示：下载正点原子串口调试助手2.8及以上版本
 [串口调试助手软件(ATK-XCOM) 版本：v3.0 — 正点原子资料下载中心 1.0.0 文档](http://47.111.11.73/docs/tool/ruanjian/ATK-XCOM.html)
4. 接线：       
CH340的VCC线 --- 无人机的VCC（左1）或者VCOM（右9）
CH340的GND线 --- 无人机的GND（左10）   
CH340的TX线     --- 无人机的UART1TX（左3）    
CH340的RX线     --- 无人机的UART1RX(左2)
    
    ![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=NjFiYzBjNmY2OWZiMDA0YTU4MTA1ZTA0Yjc5NzkwMDZfMnVLMTBHRzlrVlRva0lxNW5XVDROVzRkTlhwanRNQVlfVG9rZW46TGVSeGI4U1hmb090NEl4VWVtamNkamJ3bk9iXzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)
    如下图所示
![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=MDI5YjViZTJiMDM2N2Y5MGIwZDY2NjQ1ZWI3YzVmNDRfelVKQ1paSGxjamUzbU5JZ3VXY2ZjYkRONXlaMjFNZG5fVG9rZW46TXpRNWI4VXB0b0VyMll4cjdNb2M1ODJ4bktiXzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)
    
    ![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=NDMzMjdhYTYwOWFmMjBhOTlhZDE0MGZhYmQ2NDA2OGNfOVVxallyak5MdnR4UUZDUzM4aWJiS3VPVTNuWHBPZlZfVG9rZW46TXZqVWJDbk9ab1JjME14ekRFUWNLQ2E5bnRHXzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)
 5. 使用
安装成功后，在设备管理器->端口中可以查看到名为USB Serial Port（Com16），之后打开串口调试助手，右侧波特率选择115200，串口选择COM16，打开串口后可以看到如下界面，表示接收成功

![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=NmQ1ZDI0ZjQ0NWI0ODNmNDIyZTE2YzhhYzUyMWE3YmZfQlo2b2lNUG9ObFc2cFJYTDJrWmg4WHo0VUt2anQ1cGdfVG9rZW46VWdtVGJjeXI5bzZqNnB4eU9UZ2NDVGtabmM5XzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)

##  Cfclient（不建议的方式）
1. 代码下载：https://github.com/eurkaaaa/crazyflie-firmware/blob/debug_receive/examples/app_uart_athena/src/uart_athena.c

![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=MDViZjhjYzEwNDdhMzkwMzdkNTZlNWUzYTZkZTExZGJfSmQzSnluZWxiNk5peGJFMlhLdDFLVzRDRDFyVVhJMEhfVG9rZW46VnFUSGJxbHozb05VQ3N4dU05cmNrV0RSbjNjXzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)

2. 固件烧录：将下载后的代码按照make cload的方式烧录进无人机，用PA连接无人机后，观察cfclient界面
![](https://seunetsi.feishu.cn/space/api/box/stream/download/asynccode/?code=ZTQ4Mzc3YTQ1YzEzNjY4NTQ4NjBhNjc2MTE4MTU2MzJfbENRbjhsVEp4YjVkRjBqNmNhMFBNd3ZHVmlZOWZPYlNfVG9rZW46TENod2J3SDY4b2FDRzV4RU5QSmNnYmVtbkJoXzE3MzY2NjAzMzI6MTczNjY2MzkzMl9WNA)

>输出的时候需要将语句之间的时间间隔调大，如果较短，会出现严重字符串混乱，而这些在串口调试助手当中都没有
## 注意事项
1. 目前输出功能不支持多线程同时输出，尽量保证每次运行时候只有一个线程在执行输出操作，否则可能输出错串的现象
2. 如果出现没有打印换行符的现象，可能是正点原子串口助手版本过低，建议更换到2.8
