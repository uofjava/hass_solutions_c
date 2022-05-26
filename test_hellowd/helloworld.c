/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/init.h"
#include "board.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>


#include "aos/vfs.h"

#include <vfsdev/gpio_dev.h>
#include <drivers/char/u_device.h>
#include <drivers/u_ld.h>


int application_start(int argc, char *argv[])
{
    int count = 0;

    printf("nano entry here!\r\n");
    int fd = 0;
    int id = 22;
    int ret = 0;
    struct gpio_io_config config;
    fd = open("/dev/gpio", 0);
    printf("open gpio %s, fd:%d\r\n", fd >= 0 ? "success" : "fail", fd);
    
    
    
    while (1) {

        printf("hello world! count %d \r\n", count++);
        
        
        if (fd >= 0) {
            config.id = id;
            config.config = GPIO_IO_OUTPUT | GPIO_IO_OUTPUT_PP;
            config.data = 1;
            ret = ioctl(fd, IOC_GPIO_SET, (unsigned long)&config);
            printf("gpio write %d return %d\r\n", config.data, ret);
        }


        aos_msleep(10000);

        if (fd >= 0) {
            config.id = id;
            config.config = GPIO_IO_OUTPUT | GPIO_IO_OUTPUT_PP;
            config.data = 0;
            ret = ioctl(fd, IOC_GPIO_SET, (unsigned long)&config);
            printf("gpio write %d return %d\r\n", config.data, ret);
        }
        aos_msleep(10000);

    };
}
