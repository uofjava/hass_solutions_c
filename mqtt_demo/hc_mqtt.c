/**
 * @copyright Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <memory.h>
#include "MQTTClient.h"

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>



int mqtt_main(int argc, char** argv)
{
	int rc = 0;

	/* setup the buffer, it must big enough */
	unsigned char buf[2000];
	unsigned char readbuf[2000];

	Network n;
	MQTTClient c;

	char *host = "192.168.3.113";  //使用mosquitto搭建的linux服务器
	short port = 1883;
	const char *subTopic = "test/aiot/sub_topic";
	const char *pubTopic = "test";
	char clientId[150] = "haas";
	char username[65] = {"yyt"};
	char password[65] = {"yyt"};

	/* network init and establish network to aliyun IoT platform */
	NetworkInit(&n);

	rc = NetworkConnect(&n, host, port);
	if(rc != 0){
		printf("Network Connect failed:%d\n", rc);
		return;
	}else{
		printf("Network Connect Success!\n");
	}

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

	// rc = MQTTSubscribe(&c, subTopic, 1, "aaaa");
	// if(rc != 0){
	// 	printf("MQTT Subscribe failed:%d\n", rc);
	// 	return;
	// }else{
	// 	printf("MQTT Subscribe Success! Topic:%s\n",subTopic);
	// }

	int cnt = 0;
    unsigned int msgid = 0;
	while (1)
	{
		// MQTTYield(&c, 5000);
		aos_msleep(50);
		MQTTMessage msg = {
			QOS1,
			0,
			0,
			0,
			"Hello world",
			strlen("Hello world"),
		};
		msg.id = ++msgid;
		rc = MQTTPublish(&c, pubTopic, &msg);
		printf("MQTTPublish %d, msgid %d\n", rc, msgid);
			
	}

	printf("MQTT example exit!\n");

	MQTTDisconnect(&c);
	NetworkDisconnect(&n);

	return 0;
}

