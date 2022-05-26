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
#define Y_ADDR                  (0x1b58)/*字读取Y输出首地址 */
#define Y_len                   96       /*0~7、10~17、20~27、30~37 */
#define M7000_ADDR              (0x1b58)/*字读取M7000数据地址 */
#define M_len                   56       /*M7000~M7007*/ 
#define D7000_ADDR              (0x1b58)/*字读取D7000保存地址 */
#define D_len                   10       /*D7000~D7009*/ 
mb_status_t init_mb(){
    mb_status_t status;
    /**
     * @brief 
     * modbus主站初始化：串口号（hass100 1），波特率(9600),校验（偶校验），超时
     */
    status = mbmaster_rtu_init(&mb_master,SERIAL_PORT,SERIAL_BAUD_RATE,MB_PAR_NONE,50);
    printf("init mb init staturs is %d\n",status);
    return status;
}

int wordTobin(uint8_t value,int index){
    // printf("value sizeof: %d ,%x \r\n",value>>1, value);
    unsigned mask = 1u <<7;
    int d =0;
    for(int i =0;mask; i++,(mask >>=1)){
        printf("%d, ", value & mask?1:0);
        // d = value & mask?1:0;
    }
    printf("\r\n");
    printf("\r\n");
    return -1;
}

void recve_handler(uint8_t *buf,uint8_t len){
    // uint16_t   *register_buf;
    // /* The register length on modbus is 16 bits */
    // for(int index = 0; index<len;index++){
    //     register_buf = (uint16_t *)buf[index];
    //     printf("read X value index:%d H:%d L:%d\n",index,register_buf[0],register_buf[1]);
    // }
    for (size_t i = 0; i < len; i++)
    {
        /* code */
        printf("data %d",buf[i]);
    }
    wordTobin(buf[0],0);
    
    printf("......\r\n");
}

int application_start(int argc, char *argv[])
{
    int count = 0;
    int status = -1;
    uint8_t     len;
    uint8_t     buf[D_len*2];
    printf("nano entry here!\r\n");
     uint16_t    data_resp = 0;
    status = init_mb();
    while (1) {
        //三菱写入M 地址  OFF:0x00 0x00   ON: 0xFF 0x00                
         status = mbmaster_write_single_coil(mb_master,DEVICE1_SLAVE_ADDR_1,0,count,NULL, &data_resp, NULL,AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                if (count != data_resp) {
                    printf( "write single register error\n");
                } else {
                    printf( "write single register ok\n");
                }
            } else {
                printf( "write single register error\n");
            }
        //三菱写入D 地址 uint16
        status = mbmaster_write_single_register(mb_master,DEVICE1_SLAVE_ADDR_1,0,count,NULL, &data_resp, NULL,AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                if (count != data_resp) {
                    printf( "write single register error\n");
                } else {
                    printf( "write single register ok\n");
                }
            } else {
                printf( "write single register error\n");
            }


        // if(status == MB_SUCCESS){
        //     /**
        //      * @brief 
        //      * X输入读取
        //      */
        //     // status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, Y_ADDR,
        //     //                                      Y_len, buf, &len, AOS_WAIT_FOREVER);
        //     // if (status == MB_SUCCESS) {
        //     //     recve_handler(buf,len);
        //     // } else {
        //     //     printf("X read holding register error :%d\n",status);
        //     // }
        //     aos_msleep(20);
        //     /**
        //      * @brief 
        //      * Y输入读取
        //      */
        //     status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, Y_ADDR,
        //                                          Y_len, buf, &len, AOS_WAIT_FOREVER);
        //     if (status == MB_SUCCESS) {
        //         recve_handler(buf,len);
        //     } else {
        //         printf("M read holding register error :%d\n",status);
        //     }
        //     aos_msleep(20);
        //     /**
        //      * @brief 
        //      * M7000输入读取
        //      */
        //     status = mbmaster_read_holding_registers(mb_master, DEVICE1_SLAVE_ADDR_1, D7000_ADDR,
        //                                          D_len, buf, &len, AOS_WAIT_FOREVER);
        //     if (status == MB_SUCCESS) {
        //         recve_handler(buf,len);
        //     } else {
        //         printf("D read holding register error :%d\n",status);
        //     }
        // }
        status = MB_SUCCESS;
        printf("hello world! count %d \r\n", count++);
        aos_msleep(5000);
    };
}
