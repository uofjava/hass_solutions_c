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
#include "netmgr.h"
#include <uservice/eventid.h>
#include <uservice/uservice.h>

static int _ip_got_finished = 0;
aos_queue_t queue_handle;
aos_task_t al_task;
aos_task_t modbus_task;
int MESSAGE_MAX_LENGTH = 60;          /* X点12个字，Y点12个字，M点12个字，D点24个字*/
static uint8_t queue_buffer[60 * 30]; /* for the internal buffer of the queue */

bool isStartALiotTask = false;

extern int al_iot_main(int argc, char *argv[]);
// extern int pl_500_modbus_main(int argc, char *argv[]);
extern int pl_500_modbus_hass100_main(int argc, char *argv[]);

static void al_iot_func(void *data)
{
    printf("al task start!\r\n");
    isStartALiotTask = true;
    al_iot_main(0, NULL);
}
static void pl_500_mb_func(void *data)
{
    printf("modbus task start!\r\n");
    // pl_500_modbus_main(0,NULL);
    pl_500_modbus_hass100_main(0, NULL);
}

static void creatQueue()
{
    aos_status_t status;
    status = aos_queue_new(&queue_handle, (void *)queue_buffer, sizeof(queue_buffer),
                           MESSAGE_MAX_LENGTH);
    if (status != 0)
    {
        printf("[%s]create queue error\r\n", "pl_500");
        return;
    }
}
static void creatALiotTask()
{
    aos_status_t ret;
    ret = aos_task_create(&al_task, "linksdk_demo", al_iot_func,
                          NULL, NULL, 1024 * 8, AOS_DEFAULT_APP_PRI, AOS_TASK_AUTORUN);
    if (ret < 0)
    {
        printf("create linksdk demo task failed, ret = %ld \r\n", ret);
    }
}

static void creatModbusTask()
{
    aos_status_t ret;
    ret = aos_task_create(&modbus_task, "modbus_demo", pl_500_mb_func,
                          NULL, NULL, 1024 * 4, AOS_DEFAULT_APP_PRI, AOS_TASK_AUTORUN);
    if (ret < 0)
    {
        printf("create modbus_demo task failed, ret = %ld \r\n", ret);
    }
}

static void wifi_event_cb(uint32_t event_id, const void *param, void *context)
{
    aos_status_t ret;
    if (event_id != EVENT_NETMGR_DHCP_SUCCESS)
        return;
    if (_ip_got_finished != 0)
        return;
    _ip_got_finished = 1;
    creatQueue();
    creatALiotTask();
    creatModbusTask();
}

int application_start(int argc, char *argv[])
{

    printf("nano entry here!\r\n");

    aos_set_log_level(AOS_LL_NONE);
    event_service_init(NULL);

    netmgr_service_init(NULL);
    netmgr_set_auto_reconnect(NULL, true);
    netmgr_wifi_set_auto_save_ap(true);

    event_subscribe(EVENT_NETMGR_DHCP_SUCCESS, wifi_event_cb, NULL);
    while (1)
    {
        aos_msleep(10000);
        // printf("ip %d, aliotask : %d",_ip_got_finished,isStartALiotTask);
    };
    return 0;
}
