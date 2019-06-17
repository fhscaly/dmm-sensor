#include <stdint.h>
#include <time.h>

#include <json-c/json.h>
#include "data_format_util.h"
#include "message_protocol.h"

const char* build_json( Message_header * message ) {


    /* example JSON
     *
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
        "value": "0.0"
        }]
        }
     */

    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj,"timestamp",json_object_new_int(message->time));
    json_object_object_add(jobj,"warehouseId", json_object_new_string(message->warehouse_id));
    json_object_object_add(jobj,"assetId",json_object_new_int(message->asset_id));
    json_object_object_add(jobj,"assetName",json_object_new_string(message->asset_name));
    json_object_object_add(jobj,"eventType",json_object_new_string(message->event_type));
    json_object_object_add(jobj,"refreshInterval",json_object_new_int(message->refresh_interval));

    /* TODO:
     * loop through  message->value_count and adding new json objects to array
     * currently only one value in array values allowed
     */

    struct json_object *jobji = json_object_new_object();
    struct json_object *jarray = json_object_new_array();

    json_object_object_add(jobji,"key",json_object_new_string(message->values[0].key));
    json_object_object_add(jobji,"type",json_object_new_string(message->values[0].type));
    json_object_object_add(jobji,"value",json_object_new_string(message->values[0].value));

    json_object_array_add(jarray, jobji);

    json_object_object_add(jobj,"values",jarray);

    // free the memory
    free(message->values);
    free(message);

    return json_object_to_json_string(jobj);
}

