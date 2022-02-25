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
#include "mbmaster.h"

#define MOUDLE "modbus_master"

mb_handler_t *mb_master;
#define SERIAL_PORT             (1)    /* 串口号：1 */
#define SERIAL_BAUD_RATE        (19200) /* 波特率 */
#define DEVICE1_SLAVE_ADDR_1    (0x1)  /* 从站站号 */
#define X_ADDR                  (0x3400)/*字读取X输入首地址 */ 
#define X_len                   56       /*3GA-60MR:0~7、10~17、20~27、30~37、40~47、50~57、60~67、70~77*/
#define Y_ADDR                  (0x3300)/*字读取Y输出首地址 */
#define Y_len                   28       /*0~7、10~17、20~27、30~37 */
#define M7000_ADDR              (0x1b58)/*字读取M7000数据地址 */
#define M_len                   8       /*M7000~M7007*/ 
#define D7000_ADDR              (0x1b58)/*字读取D7000保存地址 */
#define D_len                   10       /*D7000~D7009*/ 
mb_status_t init_mb(){
    mb_status_t status;
    /**
     * @brief 
     * modbus主站初始化：串口号（hass100 1），波特率(9600),校验（偶校验），超时
     */
    status = mbmaster_rtu_init(&mb_master,SERIAL_PORT,SERIAL_BAUD_RATE,MB_PAR_EVEN,500);
    printf("init mb init staturs is %d\n",status);
    return status;
}

void recve_handler(uint8_t *buf,uint8_t len){
    uint16_t   *register_buf;
    /* The register length on modbus is 16 bits */
    for(int index = 0; index<len;index++){
        register_buf = (uint16_t *)buf[index];
        printf("read X value index:%d H:%d L:%d\n",index,register_buf[0],register_buf[1]);
    }
}

int application_start(int argc, char *argv[])
{
    int count = 0;
    int status = -1;
    uint8_t     len;
    uint8_t     buf[D_len*2];
    printf("nano entry here!\r\n");
    
    status = init_mb();
    while (1) {
        if(status == MB_SUCCESS){
            /**
             * @brief 
             * X输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, X_ADDR,
                                                 M_len, buf, &len, AOS_WAIT_FOREVER);
            if (status == MB_SUCCESS) {
                recve_handler(buf,len);
            } else {
                printf("read holding register error\n");
                status = MB_SUCCESS;
            }
            /**
             * @brief 
             * Y输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, Y_ADDR,
                                                 Y_len, buf, &len, AOS_WAIT_FOREVER);
            if (status == MB_SUCCESS) {
                recve_handler(buf,len);
            } else {
                printf("read holding register error\n");
                status = MB_SUCCESS;
            }
            /**
             * @brief 
             * M7000输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, M7000_ADDR,
                                                 M_len, buf, &len, AOS_WAIT_FOREVER);
            if (status == MB_SUCCESS) {
                recve_handler(buf,len);
            } else {
                printf("read holding register error\n");
                status = MB_SUCCESS;
            }
            /**
             * @brief 
             * M7000输入读取
             */
            status = mbmaster_read_holding_registers(mb_master, DEVICE1_SLAVE_ADDR_1, D7000_ADDR,
                                                 D_len, buf, &len, AOS_WAIT_FOREVER);
            if (status == MB_SUCCESS) {
                recve_handler(buf,len);
            } else {
                printf("read holding register error\n");
                status = MB_SUCCESS;
            }
        }
        printf("hello world! count %d \r\n", count++);
        aos_msleep(5000);
    };
}
