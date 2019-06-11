#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <mosquitto.h>
#include <json-c/json.h>
#include <time.h>

#define mqtt_host "192.168.0.115"
#define mqtt_port 1883

/* Global variables for use in callbacks. See sub_client.c for an example of
 * using a struct to hold variables for use in callbacks. */
static bool first_publish = true;
static int last_mid = -1;
static int last_mid_sent = -1;
static char *line_buf = NULL;
static int line_buf_len = 1024;
static bool connected = true;
static bool disconnect_sent = false;
static int publish_count = 0;
static bool ready_for_repeat = false;
static int refresh_interval = 60;
static int message_count = 0;

/* MQTT functions */

float get_next_values() {
    DIR *dir;
    struct dirent *dirent;
    char dev[16];      // Dev ID
    char devPath[128]; // Path to device
    char buf[256];     // Data from device
    char tmpData[6];   // Temp C * 1000 reported by device
    char path[] = "/sys/bus/w1/devices";
    ssize_t numRead;

    dir = opendir (path);
    if (dir != NULL)
    {
        while ((dirent = readdir (dir)))
            // 1-wire devices are links beginning with 28-
            if (dirent->d_type == DT_LNK &&
                strstr(dirent->d_name, "28-") != NULL) {
                strcpy(dev, dirent->d_name);
                printf("\nDevice: %s\n", dev);
            }
        (void) closedir (dir);
    }
    else
    {
        perror ("Couldn't open the w1 devices directory");
        return 1;
    }

    // Assemble path to OneWire device
    sprintf(devPath, "%s/%s/w1_slave", path, dev);

    // Read temp continuously
    // Opening the device's file triggers new reading
    int fd = open(devPath, O_RDONLY);
    if(fd == -1)
    {
        perror ("Couldn't open the w1 device.");
        return 1;
    }
    while((numRead = read(fd, buf, 256)) > 0)
    {
        strncpy(tmpData, strstr(buf, "t=") + 2, 5);
        return strtof(tmpData, NULL);
    }
    close(fd);
    return 0;
}


char* my_payload( )
{
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

    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj,"timestamp",json_object_new_string(ctime((const time_t *) time(NULL))));
    json_object_object_add(jobj,"warehouseId", json_object_new_string("XX00815"));
    json_object_object_add(jobj,"assetId",json_object_new_int(-1));
    json_object_object_add(jobj,"assetName",json_object_new_string("temp1"));
    json_object_object_add(jobj,"eventType",json_object_new_string("temp_reading"));
    json_object_object_add(jobj,"refreshInterval",json_object_new_int(refresh_interval));

    struct json_object *jobj2 = json_object_new_object();

    json_object_object_add(jobj2,"key",json_object_new_string("temperature"));
    json_object_object_add(jobj2,"type",json_object_new_string("FLOAT"));
    json_object_object_add(jobj2,"value",json_object_new_double((get_next_values())));

    json_object_object_add(jobj,"values",jobj2);

    return (char *) json_object_to_json_string(jobj);
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{

	printf("%s\n", str);
}


void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
    fprintf(stderr, "DISCONNECT CALLBACK!.\n");
    connected = false;
}

int my_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, void *payload, int qos, bool retain)
{
    ready_for_repeat = false;
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
        ready_for_repeat = true;
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
