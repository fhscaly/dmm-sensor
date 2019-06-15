#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


//#include "C:\Program Files\mosquitto\devel\mosquitto.h"
//#include "D:\json-c\json.h"
#include <mosquitto.h>
#include <json-c/json.h>
#include <stdbool.h>

#include "message_protocol.h"
#include "data_format_util.h"
#include "sensor.h"

#define mqtt_host "192.168.0.115"
#define mqtt_port 1883

/* Global variables for use in callbacks. See sub_client.c for an example of
 * using a struct to hold variables for use in callbacks. */
static bool first_publish = true;
static int last_mid_sent = -1;
static bool disconnect_sent = false;
static int publish_count = 0;
static int refresh_interval = 60;
static int message_count = 0;


const char* my_payload( ) {
    /*
     * payload = {
        "timestamp": 0,
        "warehouseId": "XX00815",
        "assetId": "-1",
        "assetName": "temp1",
        "eventType": "temp_reading",
        "refreshInterval": -1,
        "values": [{
        "key": "temperature",
        "type": "DOUBLE",
        "value": 0.0
        }]
        }
     */


    Message_header *message = create_message_header("FH00017", -1, "HOME:TS001", "PUBLISH", 30);

    //double temp_data = get_next_value_from_sensor();
    //add_value_to_message(message, "data", "DOUBLE", &temp_data);
    add_value_to_message(message, "data", "DOUBLE", 25);

    return build_json(message);
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{

	printf("%s\n", str);
}


void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    fprintf(stderr, "DISCONNECT CALLBACK!.\n");

}

int my_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, void *payload, int qos, bool retain)
{
    if( first_publish == false){
	    return mosquitto_publish(mosq, mid, NULL, payloadlen, payload, qos, retain);
    }else{
        first_publish = false;
        return mosquitto_publish(mosq, mid, topic, payloadlen, payload, qos, retain);
    }
}


void my_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags )
{
    fprintf(stderr, "CONNECT CALLBACK!.\n");
    int rc = MOSQ_ERR_SUCCESS;

    if(!result){

        char * message = my_payload();
                rc = my_publish(mosq, &message_count, "SENSOR_DATA", strlen(message) , message, 0, true);
                message_count++;
        }

        if(rc){
                switch(rc){
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
    fprintf(stderr, "PUBLISH CALLBACK!.\n");

    last_mid_sent = mid;
    if(reason_code > 127){
        fprintf(stderr, "Warning: Publish %d failed: %d.\n", mid, reason_code);
    }
    publish_count++;

    if(publish_count < 5){
    }else if(disconnect_sent == false){
        mosquitto_disconnect(mosq);
        disconnect_sent = true;
    }
}


int main (void) {


    // MQT Handling
    char clientid[24];
    struct mosquitto *mosq;
    int keepalive = 60;

    // inital msoquitto lib
    mosquitto_lib_init();

    // set client as process-id
    memset(clientid, 0, 24);
    snprintf(clientid,23,"client_%d", getpid());

    // Client constructir
    mosq = mosquitto_new(clientid, true, 0);

    if ( mosq) {

	mosquitto_log_callback_set(mosq, my_log_callback);
        mosquitto_connect_callback_set(mosq, my_connect_callback);
        mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
        mosquitto_publish_callback_set(mosq, my_publish_callback);

        if(mosquitto_connect(mosq, "localhost", mqtt_port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
	   mosquitto_lib_cleanup();
		return 1;
	}

	mosquitto_loop(mosq,100,-1 );

	while(1) {
	  const char * message = my_payload();
	  //my_publish(mosq, 1, "SENSOR_DATA", 5, "HELLOO", 0, true);
	  //my_publish(mosq, &last_mid_sent, "SENSOR_DATA", strlen(message) , message, 0, true);
	  mosquitto_publish(mosq, &last_mid_sent, "SENSOR_DATA", strlen(message) , message, 1, false);
	  //mosquitto_publish(mosq, &last_mid_sent, "SENSOR_DATA", 5 , "HELLO", 1, false);

	  last_mid_sent++;
	  sleep(3);


	//mosquitto_publish(mosq, &message_count, "SENSOR_DATA", 5, "HELLO", 0, true);
	}

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
        fprintf(stderr, "ENDE!!!!!!!!!!!!!!!!!!!!!!!.\n");
        return 0;
	//mosquitto_loop_forever(mosq, -1, 1); 

   } else
    {
        fprintf(stderr, "No msq connection established.\n");
        mosquitto_lib_cleanup();
    }
}
