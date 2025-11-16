# 最终效果
<img src="./pic/Preview.png" alt="最终效果" style="width:50%;">

使用 Qt 6.9.3 实现的一个模拟数据采集、处理的并且实时显示的 demo 。


## 1.1 已经实现功能
每秒采样 51200 * 32 个数据   \
自定义的高精度定时器，实现每秒采集 51200 次数据  \
滑动窗口均值滤波，默认100ms  \
使用 QCustomplot 绘制结果  \
原始数据写入csv文件。 写入过程可以暂停  

## 1.2 TODO
实时修改滤波参数


# 3 实现过程：
使用QTimer每4ms发送一次信号，DemoProcessor会采集(512000 * 4 / 1000)个数据, 将采集的原始数据发送给 CSV_Saver线程，使用均值滤波处理原始数据， QTimer每40ms发送一个信号给MainWindow来，将处理后的数据使用 QCustomplot 显示， \




