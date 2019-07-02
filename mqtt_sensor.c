#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#include <mosquitto.h>
#include <json-c/json.h>

#include "message_protocol.h"
#include "data_format_util.h"
#include "sensor.h"

#define mqtt_topic "/sensors/HOME:TS001"
#define mqtt_port 1883


static int last_mid_sent = 0;
static int refresh_interval = 1;

const char* my_payload( ) {

    Message_header *message = create_message_header("FH00017", -1, "HOME:TS001", "PUBLISH", refresh_interval);

    float temp_data = sensor_read();
    
    // convert back to string - since mqtt protocol require string
    char value[10];
    snprintf(value, sizeof(value), "%f", temp_data);

    add_value_to_message(message, "data", "FLOAT", value);

    return build_json(message);
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{

}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags )
{
    //successful connected, publish message
    const char * message = my_payload();
    mosquitto_publish(mosq, &last_mid_sent, mqtt_topic, strlen(message) , message, 1, false);
    last_mid_sent++;

    if(result){
        switch(result){
            case MOSQ_ERR_INVAL:
                fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
                break;
                case MOSQ_ERR_NOMEM:
                    fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
                    break;
                case MOSQ_ERR_NO_CONN:
                    fprintf(stderr, "Error: Client not connected when trying to publish.\n");
                    break;
                case MOSQ_ERR_PROTOCOL:
                    fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
                    break;
                case MOSQ_ERR_PAYLOAD_SIZE:
                    fprintf(stderr, "Error: Message payload is too large.\n");
                    break;
        }
        mosquitto_disconnect(mosq);
    }
}

void my_publish_callback(struct mosquitto *mosq, void *obj, int mid, int reason_code )
{

    //successful published message, trigger next every seconds
    sleep(refresh_interval);
    const char * message = my_payload();
    mosquitto_publish(mosq, &last_mid_sent, mqtt_topic, strlen(message) , message, 1, false);
    last_mid_sent++;
}

int main(int argc, char* argv[]) {

    if ( argc  < 2 || argc > 3  ) {
        printf("wrong usage: arg1 = MQTT broker IP, optional parameter arg2 = MQTT Port \n");
	return 1;
    }

    int port;

    if ( argc == 3) {
      port = atoi(argv[2]);
    } else {
      port = mqtt_port;
   } 
       

    // MQT Handling
    char clientid[24];
    struct mosquitto *mosq;
    int keepalive = 30;

    // inital msoquitto lib
    mosquitto_lib_init();

    // set client as process-id
    memset(clientid, 0, 24);
    snprintf(clientid,23,"client_%d", getpid());

    // Client constructor
    mosq = mosquitto_new(clientid, true, 0);

    if ( mosq) {

	mosquitto_log_callback_set(mosq, my_log_callback);
        mosquitto_connect_callback_set(mosq, my_connect_callback);
        mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
        mosquitto_publish_callback_set(mosq, my_publish_callback);

        if(mosquitto_connect(mosq, argv[1], port, keepalive)){
		    fprintf(stderr, "Unable to connect.\n");
	        mosquitto_lib_cleanup();
		return 1;
	    }

        mosquitto_loop_forever(mosq, 1000, 1);
    }
}
