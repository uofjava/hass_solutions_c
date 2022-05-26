/**
 * @copyright Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <memory.h>
#include "MQTTClient.h"

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "led.h"



Network n;
MQTTClient c;
unsigned char buf[2000];
unsigned char readbuf[2000];
char *host = "192.168.3.113";  //使用mosquitto搭建的linux服务器
short port = 1883;
const char *subTopic = "test/aiot/sub_topic";
const char *pubTopic = "028_2022";
char clientId[150] = "haas";
char username[65] = {"yyt"};
char password[65] = {"yyt"};
int rc = 0;
int netInit()
{
	NetworkInit(&n);
	rc = NetworkConnect(&n, host, port);
	if(rc != 0){
		printf("Network Connect failed:%d\n", rc);
		return -1;
	}else{
		printf("Network Connect Success!\n");
	}
	return 0;
}
int initMMQTT()
{
	/* init mqtt client */
	MQTTClientInit(&c, &n, 1000, buf, sizeof(buf), readbuf, sizeof(readbuf));
	/* set mqtt connect parameter */
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = clientId;
	data.username.cstring = username;
	data.password.cstring = password;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	printf("Connecting to %s:%d\n", host, port);
	rc = MQTTConnect(&c, &data);
	if(rc != 0){
		printf("MQTT Connect server failed:%d\n", rc);
		return;
	}else{
		printf("MQTT Connect Success!\n");
	}
	return 0;
}



int mqtt_main(int argc, char** argv)
{
	int back = netInit();
	while (back == -1)
	{
		led_switch(2, LED_ON);
		/* code */
		back = netInit();
		aos_msleep(1000);
	}
	led_switch(2, LED_OFF);
	initMMQTT();
	int cnt = 0;
    unsigned int msgid = 0;
	unsigned int prodeck = 0;
	while (1)
	{
		// MQTTYield(&c, 5000);
		aos_msleep(3000);
		char *send_alio_data = (char *)calloc(120,sizeof(char));
        strcat(send_alio_data,"{");
		char d [30];
		msgid = rand()%5 +1;
		sprintf(d,"\"设备状态\": %d,",msgid);
		strcat(send_alio_data,d);

		sprintf(d,"\"异常\": %d,",0);
		strcat(send_alio_data,d);
		
		sprintf(d,"\"异常类型\": \"气压\",");
		strcat(send_alio_data,d);
		

		sprintf(d,"\"生产总数\": %d,",msgid);
		strcat(send_alio_data,d);
		

		sprintf(d,"\"设备code\": \"028_1_%d\"",msgid);
		strcat(send_alio_data,d);
		



		strcat(send_alio_data, "}");
		MQTTMessage msg = {
			QOS1,
			0,
			0,
			0,
			send_alio_data,
			strlen(send_alio_data),
		};
		msg.id = ++msgid;
		// printf(send_alio_data);
		rc = MQTTPublish(&c, pubTopic, &msg);
		printf("MQTTPublish %d, msgid %d\n", rc, msgid);
		aos_free(send_alio_data);
		if(rc ==-1){
			MQTTDisconnect(&c);
			NetworkDisconnect(&n);
			back = netInit();
			while (back == -1)
			{
				led_switch(2, LED_ON);
				/* code */
				back = netInit();
				aos_msleep(1000);
			}
			led_switch(2, LED_OFF);
			initMMQTT();
		}
	}
	printf("MQTT example exit!\n");
	MQTTDisconnect(&c);
	NetworkDisconnect(&n);
	return 0;
}

