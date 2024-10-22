使用教程

配置硬件I2C，代码用的I2C1

头文件引用

```
#include "mpu6050.h"
```

添加结构体

```
MPU6050_t MPU6050;
```

在main函数里，初始化为

```
while(MPU6050_Init(&hi2c1) == 1);
```

while(1)里

```
MPU6050_Read_All(&hi2c1, &MPU6050);
HAL_Delay(100);
```

提取数据

```
MPU6050.*
```

*为结构体里的内容



原github

```
https://github.com/leech001/MPU6050
```
