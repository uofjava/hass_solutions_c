#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <aos/kernel.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_dm_api.h"
#include <cJSON.h>

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

extern aos_queue_t queue_handle;
extern aos_queue_t queue_handle_iot_down;
uint8_t    iot_message_buf[6] = {0x01,0x06,0x00,0x00,0x00,0x00};
uint8_t adress[2];
uint8_t data_value[2];
extern int MESSAGE_MAX_LENGTH;



/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] mqtt_basic_demo&a13FN5TplKq
 *
 * 上面这条日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h
 *
 */

/* 日志回调函数, SDK的日志会从这里输出 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    // printf("%s", message);
    return 0;
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
        }
        break;

        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        // printf("demo_mqtt_process_thread\r\n");
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
        }
        // printf("demo_mqtt_recv_thread\r\n");
    }
    return NULL;
}

/* 用户数据接收处理回调函数 */
static void demo_dm_recv_handler(void *dm_handle, const aiot_dm_recv_t *recv, void *userdata)
{
    printf("demo_dm_recv_handler, type = %d\r\n", recv->type);
    cJSON *params = NULL;
   
    switch (recv->type) {
        /* 属性上报, 事件上报, 获取期望属性值或者删除期望属性值的应答 */
        // case AIOT_DMRECV_GENERIC_REPLY: {
        //     printf("msg_id = %d, code = %d, data = %.*s, message = %.*s\r\n",
        //            recv->data.generic_reply.msg_id,
        //            recv->data.generic_reply.code,
        //            recv->data.generic_reply.data_len,
        //            recv->data.generic_reply.data,
        //            recv->data.generic_reply.message_len,
        //            recv->data.generic_reply.message);
        // }
        // break;

        /* 属性设置 */
        case AIOT_DMRECV_PROPERTY_SET: {
            printf("msg_id = %ld, params = %.*s\r\n",
                   (unsigned long)recv->data.property_set.msg_id,
                   recv->data.property_set.params_len,
                   recv->data.property_set.params);
            params = cJSON_Parse(recv->data.property_set.params);
            if (params == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                    printf(stderr, "Error before: %s\n", error_ptr);

                cJSON_Delete(params);
                break;
            }  
            adress[0] =  atoi(params->child->string);
            adress[1] =  atoi(params->child->string) >> 8;
            data_value[0] = params->child->valueint;
            data_value[1] = params->child->valueint >> 8;
            
            // printf("index: %d, data:%d\r\n",index,data_value);
            if( (atoi(params->child->string) > 17) &&  (atoi(params->child->string) < 30)){
                iot_message_buf[1] = 0x06;
                iot_message_buf[2] = adress[1];
                iot_message_buf[3] = adress[0];
                iot_message_buf[4] = data_value[1];
                iot_message_buf[5] = data_value[0];
            }
            printf("%o,%o,%o,%o, \r\n",adress[0],adress[1],data_value[0],data_value[1]);
          
            aos_status_t status = aos_queue_send(&queue_handle_iot_down, (void *)iot_message_buf, sizeof(iot_message_buf));
            if (status != 0) {
                printf("[%s]send iot buf  queue error :%d \r\n", "pl_500_iot ",status);
            }

            // printf("%s\r\n",params->child->string);
            // printf("%d\r\n",params->child->valueint);
        }
        break;
        default:
            break;
    }
}

/* 属性上报函数演示 */
int32_t demo_send_property_post(void *dm_handle, char *params)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_PROPERTY_POST;
    msg.data.property_post.params = params;

    return aiot_dm_send(dm_handle, &msg);
}

/* 事件上报函数演示 */
int32_t demo_send_event_post(void *dm_handle, char *event_id, char *params)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_EVENT_POST;
    msg.data.event_post.event_id = event_id;
    msg.data.event_post.params = params;

    return aiot_dm_send(dm_handle, &msg);
}

/* 演示了获取属性LightSwitch的期望值, 用户可将此函数加入到main函数中运行演示 */
int32_t demo_send_get_desred_requset(void *dm_handle)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_GET_DESIRED;
    msg.data.get_desired.params = "[\"LightSwitch\"]";

    return aiot_dm_send(dm_handle, &msg);
}

/* 演示了删除属性LightSwitch的期望值, 用户可将此函数加入到main函数中运行演示 */
int32_t demo_send_delete_desred_requset(void *dm_handle)
{
    aiot_dm_msg_t msg;

    memset(&msg, 0, sizeof(aiot_dm_msg_t));
    msg.type = AIOT_DMMSG_DELETE_DESIRED;
    msg.data.get_desired.params = "{\"LightSwitch\":{}}";

    return aiot_dm_send(dm_handle, &msg);
}


int haxTobin(uint8_t value,int index){
    // printf("value sizeof: %d ,%x \r\n",value>>1, value);
    unsigned mask = 1u <<7;
    int d = 0;
    for(int i =0;mask; i++,(mask >>=1)){
        d = value & mask?1:0;
        if((7-index) == i){
            // printf("get data %d index %d\r\n",d, 7-index);
            return d;
        }
    }
    return -1;
}
    
int al_iot_main(int argc, char *argv[])
{
    int32_t     res = STATE_SUCCESS;
    void       *dm_handle = NULL;
    void       *mqtt_handle = NULL;
    char       *url = "iot-as-mqtt.cn-shanghai.aliyuncs.com"; /* 阿里云平台上海站点的域名后缀 */
    char        host[100] = {0}; /* 用这个数组拼接设备连接的云平台站点全地址, 规则是 ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com */
    uint16_t    port = 443;      /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* TODO: 替换为自己设备的三元组 */
    char *product_key       = "gic7M0DT51b";
    char *device_name       = "PL_500_test";
    char *device_secret     = "5836a1c5b1f1be9554c8c084d9cd109c";

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    // aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    snprintf(host, 100, "%s.%s", product_key, url);
    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 创建DATA-MODEL实例 */
    dm_handle = aiot_dm_init();
    if (dm_handle == NULL) {
        printf("aiot_dm_init failed");
        return -1;
    }
    /* 配置MQTT实例句柄 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_MQTT_HANDLE, mqtt_handle);
    /* 配置消息接收处理回调函数 */
    aiot_dm_setopt(dm_handle, AIOT_DMOPT_RECV_HANDLER, (void *)demo_dm_recv_handler);

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = aos_task_new("demo_mqtt_process", demo_mqtt_process_thread, mqtt_handle, 4096);
    // res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res != 0) {
        printf("create demo_mqtt_process_thread failed: %d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = aos_task_new("demo_mqtt_process", demo_mqtt_recv_thread, mqtt_handle, 4096);
    // res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res != 0) {
        printf("create demo_mqtt_recv_thread failed: %d\n", res);
        return -1;
    }

    uint8_t       message_buf[MESSAGE_MAX_LENGTH];
    size_t        rev_size = 0;
    uint32_t      i;
    aos_status_t  status;
    
    /* 主循环进入休眠 */
    while (1) {
        /* TODO: 以下代码演示了简单的属性上报和事件上报, 用户可取消注释观察演示效果 */
        status = aos_queue_recv(&queue_handle, 10, (void *)message_buf, &rev_size);
        if (status == 0) {
            /* show message data */
            printf("[%s] %d recv message \r\n", "al_iot:", rev_size);
            char *send_alio_data = (char *)calloc(400,sizeof(char));
            strcat(send_alio_data,"{");
            for (i = 0; i < rev_size /2; i++) {
                char d [10];
                sprintf(d,"\"%d\": %d,",i, message_buf[2*i]*256 + message_buf[2*i+1]);
                strcat(send_alio_data,d);
                // printf("index :%d,value %d \r\n",i, (message_buf[2*i] + message_buf[2*i+1]*256));
            }
            // 添加上下气缸报警M7000
            char d [10];
            sprintf(d,"\"30\": %d,",haxTobin(message_buf[0],0));
            strcat(send_alio_data,d);
            //添加掉料停止报警M7001
            sprintf(d,"\"31\": %d,",haxTobin(message_buf[0],1));
            strcat(send_alio_data,d);
            // 添加移动限位报警M7002
            sprintf(d,"\"32\": %d,",haxTobin(message_buf[0],2));
            strcat(send_alio_data,d);
            // 添加伺服驱动报警M7003
            sprintf(d,"\"33\": %d,",haxTobin(message_buf[0],3));
            strcat(send_alio_data,d);
            // 添加启动M7116 M7100~M7115:2个字，
            sprintf(d,"\"34\": %d,",haxTobin(message_buf[14],0));
            strcat(send_alio_data,d);
            // 添加停止M7117 M7100~M7115:1 M7117
            sprintf(d,"\"35\": %d,",haxTobin(message_buf[14],1));
            strcat(send_alio_data,d);
            // 添加急停M7118 M7100~M7115:1 M7118
            sprintf(d,"\"36\": %d,",haxTobin(message_buf[14],2));
            strcat(send_alio_data,d);
            //添加故障M7215
            sprintf(d,"\"37\": %d,",haxTobin(message_buf[25],7));
            strcat(send_alio_data,d);
            // 添加产量 D7000
            sprintf(d,"\"38\": %d",message_buf[2*18]*256 + message_buf[2*18+1]);
            strcat(send_alio_data,d);
            strcat(send_alio_data, "}");
            demo_send_property_post(dm_handle, send_alio_data);
             //释放内存
            aos_free(send_alio_data);
            // send_alio_data = NULL;
        } else {
            printf("[%s]recv buf queue error\n", "al_iot:");
        }
        aos_msleep(5000);
    }
    
    return 0;
}

