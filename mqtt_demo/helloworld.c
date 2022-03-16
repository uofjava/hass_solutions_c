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
#include <uservice/uservice.h>
#include <uservice/eventid.h>



extern int mqtt_main(int argc, char *argv[]);

static int _ip_got_finished = 0;

static void entry_func(void *data)
{
    printf("start mqtt main");
    mqtt_main(0 , NULL);

}


static void wifi_event_cb(uint32_t event_id, const void *param, void *context)
{
    aos_task_t task;
    aos_status_t ret;
    if (event_id != EVENT_NETMGR_DHCP_SUCCESS)
        return;

    if (_ip_got_finished != 0)
        return;

    _ip_got_finished = 1;
    ret = aos_task_create(&task, "mqtt_demo", entry_func,
                          NULL, NULL, 6048, AOS_DEFAULT_APP_PRI, AOS_TASK_AUTORUN);
    if (ret < 0) {
        printf("create linksdk demo task failed, ret = %ld \r\n", ret);
    }
}





int application_start(int argc, char *argv[])
{
    int count = 0;

    printf("nano entry here!\r\n");

    aos_set_log_level(AOS_LL_DEBUG);


    
    event_service_init(NULL);

    netmgr_service_init(NULL);
    // netmgr_set_auto_reconnect(NULL, true);
    // netmgr_wifi_set_auto_save_ap(true);


    event_subscribe(EVENT_NETMGR_DHCP_SUCCESS, wifi_event_cb, NULL);
    while (1) {
        aos_msleep(1000);

    };
}
