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
#define X_ADDR                  (0x1bbc)/*字读取X输入首地址 M7100~M7195*/ 
#define X_len                   96       /*3GA-60MR:0~7、10~17、20~27、30~37、40~47、50~57、60~67、70~77*/
#define Y_ADDR                  (0x1c20)/*字读取Y输出首地址 M7200~M7295 */
#define Y_len                   96       /*0~7、10~17、20~27、30~37 */
#define M7000_ADDR              (0x1b58)/*字读取M7000数据地址 */
#define M_len                   96       /*M7000~M7095  6*2*8=96位 */ 
#define D7000_ADDR              (0x1b58)/*字读取D7000保存地址 */
#define D_len                   12       /*D7000~D70011*/ 
extern aos_queue_t queue_handle;
extern aos_task_t al_task;
extern int MESSAGE_MAX_LENGTH;
extern int isStartALiotTask;
uint8_t    message_buf[60];
mb_status_t init_mb(){
    mb_status_t status;
    /**
     * @brief 
     * modbus主站初始化：串口号（hass100 1），波特率(19200),校验（偶校验），超时
     */
    status = mbmaster_rtu_init(&mb_master,SERIAL_PORT,SERIAL_BAUD_RATE,MB_PAR_EVEN,20);
    printf("init mb init staturs is %d\n",status);
    return status;
}

int printhaxTobin(uint8_t value,int index){
    // printf("value sizeof: %d ,%x \r\n",value>>1, value);
    unsigned mask = 1u <<7;
    for(int i =0;mask; i++,(mask >>=1)){
        printf("%d,",value & mask?1:0);
    }
    printf("\r\n");
    return -1;
}

void recve_handler_first_H(uint8_t *buf, uint8_t len, int index){
    for (size_t i = index; i < index+len; i++)
    {
        message_buf[2*i] = buf[2*(i - index)+1];
        message_buf[2*i+1] = buf[2*(i - index)];
    }
    
}

void recve_handler(uint8_t *buf, uint8_t len, int index){
    for(int i = index; i < index+len; i++){
        message_buf[i] = buf[i - index];
    }
}

int pl_500_modbus_hass100_main(int argc, char *argv[])
{
    int count = 0;
    int status = -1;
    uint8_t     len;
    uint8_t     D_buf[D_len*2];
    uint8_t     M_buf[D_len*2];
    uint8_t     X_buf[D_len*2];
    uint8_t     Y_buf[D_len*2];
    printf("modbus here!\r\n");
    status = init_mb();
    while (1) {
        if(status == MB_SUCCESS){
            // memset(M_buf, 0, D_len/2);
            /**
             * @brief 
             * M7000输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, M7000_ADDR,
                                                 M_len, M_buf, &len, AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                //读取两次，M_buf为上次的值(D_buf)
                printf("获取D 的值：%d",len);
                recve_handler(M_buf,24,36);
                // recve_handler_first_H(M_buf,24/2,36/2);
                // aos_msleep(20);
                status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, M7000_ADDR,
                                                 M_len, M_buf, &len, AOS_NO_WAIT);
                if (status == MB_SUCCESS) {
                    printf("获取M 的值：%d",len);
                    // printf("************\r\n");
                    // for (size_t i = 0; i < 12; i++)
                    // {
                    //     /* code */
                    //     printhaxTobin(M_buf[i],0);
                    // }
                    // printf("***************\r\n");
                    recve_handler(M_buf,12,0);
                    // recve_handler_first_H(M_buf,12/2,0);
                } else {
                    printf("M read holding register error\n");
                }
            } else {
                printf("M read holding register error\n");
            }
            // memset(X_buf, 0, D_len/2);
            /**
             * @brief 
             * X输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, X_ADDR,
                                                 X_len, X_buf, &len, AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                // aos_msleep(20);
                printf("获取M 的值：%d",len);
                // recve_handler(X_buf,12,0);
                // recve_handler_first_H(X_buf,12/2,0);
            } else {
                printf("X read holding register error\n");
            }
            // memset(Y_buf, 0, D_len/2);
            /**
             * @brief 
             * Y输入读取
             */
            status = mbmaster_read_coils(mb_master, DEVICE1_SLAVE_ADDR_1, Y_ADDR,
                                                 Y_len, Y_buf, &len, AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                // aos_msleep(20);
                printf("获取X 的值：%d",len);
                recve_handler(Y_buf,12,12);
                // recve_handler_first_H(Y_buf,12/2,12/2);
            } else {
                printf("Y read holding register error\n");
            }
            // memset(D_buf, 0, D_len*2);
            /**
             * @brief 
             * D7000输入读取
             */
            status = mbmaster_read_holding_registers(mb_master, DEVICE1_SLAVE_ADDR_1, D7000_ADDR,
                                                 D_len, D_buf, &len, AOS_NO_WAIT);
            if (status == MB_SUCCESS) {
                printf("获取Y 的值：%d",len);
                // printf("************\r\n");
                // for (size_t i = 0; i < 12; i++)
                // {
                //     /* code */
                //     printhaxTobin(D_buf[i],0);
                // }
                // printf("***************\r\n");
                // recve_handler(D_buf,12,24);

                recve_handler_first_H(D_buf,12/2,24/2);
            } else {
                printf("D read holding register error\n");
            }
            // for (size_t i = 0; i < 60; i++)
            // {
            //     /* code */
            //     printf("%d,",message_buf[i]);
            //     if(i%12 ==0){
            //         printf("\r\n");
            //     }
            // }
            // printf("\r\n");
            status = aos_queue_send(&queue_handle, (void *)message_buf, sizeof(message_buf));
            if (status != 0) {
                printf("[%s]send buf  queue error :%d \r\n", "pl_500_modbus ",status);
            }
            status = MB_SUCCESS;
        }
        aos_msleep(20);
    };
}