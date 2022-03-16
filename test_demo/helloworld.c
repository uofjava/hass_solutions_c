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




#define MODULE_NAME "aos_queue_example"

/* taskA parameters */
#define TASKA_NAME      "taskA"
#define TASKA_PRIO       31
#define TASKA_STACKSIZE 1024

/* taskB parameters */
#define TASKB_NAME      "taskB"
#define TASKB_PRIO       30
#define TASKB_STACKSIZE 1024

/* queue resource */
#define MESSAGE_MAX_LENGTH 10 /* maximum message length */

/* Static memory for static creation */
static aos_queue_t queue_handle;                          /* queue handle */
static char        queue_buffer[MESSAGE_MAX_LENGTH * 10]; /* for the internal buffer of the queue */


/* Static memory for static creation */
static aos_queue_t queue2_handle;                          /* queue handle */
static char        queue2_buffer[MESSAGE_MAX_LENGTH * 10]; /* for the internal buffer of the queue */



/* task entry for taskA*/
static void taskA_entry(void *arg)
{
    uint32_t      i;
    aos_status_t  status;

    char     message_buf[MESSAGE_MAX_LENGTH]; /* buffer used to send message */
    char          message_rec_buf[MESSAGE_MAX_LENGTH];
    size_t        rev_size = 0;

    while (1) {
        status = aos_queue_recv(&queue2_handle,10,(void *)message_rec_buf,&rev_size);
        if (status == 0) {
            /* show message data */
            printf("[%s]%d recv2 message : ", MODULE_NAME, rev_size);
            for (i = 0; i < rev_size; i++) {
                printf("%d", message_rec_buf[i]);
            }
            printf("\r\n");
        } else {
            printf("[%s]recv2 buf queue error\n", MODULE_NAME);
        }

        for (i = 0; i < sizeof(message_buf); i++) {
            message_buf[i] = 2;
        }
        /* send message. The message length must not exceed the maximum message length */
        status = aos_queue_send(&queue_handle, (void *)message_buf, sizeof(message_buf));
        if (status != 0) {
            printf("[%s]send buf queue error\n", MODULE_NAME);
        }

        aos_msleep(1000); /* sleep 1000ms */
    }
}

/* task entry for taskB*/
static void taskB_entry(void *arg)
{
    uint32_t      i;
    aos_status_t  status;

    char     message_send_buf[MESSAGE_MAX_LENGTH]; /* buffer used to send message */


    /* The buffer must be greater than or equal to the maximum message length */
    char          message_buf[MESSAGE_MAX_LENGTH];
    size_t        rev_size = 0;

    while (1) {
        /**
         * receive message. The task will wait until it receives the message.
         * rev_size is set to the actual length of the received message.
         */
        status = aos_queue_recv(&queue_handle, 10, (void *)message_buf, &rev_size);
        if (status == 0) {
            /* show message data */
            printf("[%s]%d recv message : ", MODULE_NAME, rev_size);
            for (i = 0; i < rev_size; i++) {
                printf("%d", message_buf[i]);
            }
            printf("\r\n");
        } else {
            printf("[%s]recv buf queue error\n", MODULE_NAME);
        }


        for (i = 0; i < sizeof(message_send_buf); i++) {
            message_send_buf[i] = 1;
        }
        /* send message. The message length must not exceed the maximum message length */
        status = aos_queue_send(&queue2_handle, (void *)message_send_buf, sizeof(message_send_buf));
        if (status != 0) {
            printf("[%s]send2 buf queue error\n", MODULE_NAME);
        }

        aos_msleep(1000); /* sleep 1000ms */

    }
}



int application_start(int argc, char *argv[])
{
    int count = 0;

    printf("nano entry here!\r\n");

     aos_status_t  status;
    aos_task_t    taskA_handle;
    aos_task_t    taskB_handle;

    /**
     * create a queue.
     * queue:   queue_handle(aos_queue_t struct variable)
     * buf:     queue_buffer(for the internal buffer of the queue)
     * size:    sizeof(queue_buffer) is the length of buf
     * max_msg: MESSAGE_MAX_LENGTH(maximum message length, here is 10 byte)
     */
    status = aos_queue_new(&queue_handle, (void *)queue_buffer, sizeof(queue_buffer),
                           MESSAGE_MAX_LENGTH);
    if (status != 0) {
        printf("[%s]create queue error\n", MODULE_NAME);
        return;
    }

    status = aos_queue_new(&queue2_handle, (void *)queue2_buffer, sizeof(queue2_buffer),
                           MESSAGE_MAX_LENGTH);
    if (status != 0) {
        printf("[%s]create queue2 error\n", MODULE_NAME);
        return;
    }


    /* TaskA is a producer that produces a set of data every second. */
    status = aos_task_create(&taskA_handle, TASKA_NAME, taskA_entry, NULL, NULL, TASKA_STACKSIZE, TASKA_PRIO, AOS_TASK_AUTORUN);
    if (status != 0) {
        aos_queue_free(&queue_handle);
        printf("[%s]create %s error\n", MODULE_NAME, TASKA_NAME);
        return;
    }

    /* TaskB is the consumer that processes the data sent by taskA. */
    status = aos_task_create(&taskB_handle, TASKB_NAME, taskB_entry, NULL, NULL, TASKB_STACKSIZE, TASKB_PRIO, AOS_TASK_AUTORUN);
    if (status != 0) {
        aos_queue_free(&queue_handle);
        printf("[%s]create %s error\n", MODULE_NAME, TASKB_NAME);
        return;
    }



    while (1) {
        printf("hello world! count %d \r\n", count++);
        aos_msleep(10000);
    };
}
