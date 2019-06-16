#include <stdint.h>
#include <time.h>

#include <json-c/json.h>
#include "data_format_util.h"
#include "message_protocol.h"

const char* build_json( Message_header * message ) {

    struct json_object *jobj = json_object_new_object();

    json_object_object_add(jobj,"timestamp",json_object_new_int(message->time));
    json_object_object_add(jobj,"warehouseId", json_object_new_string(message->warehouse_id));
    json_object_object_add(jobj,"assetId",json_object_new_int(message->asset_id));
    json_object_object_add(jobj,"assetName",json_object_new_string(message->asset_name));
    json_object_object_add(jobj,"eventType",json_object_new_string(message->event_type));
    json_object_object_add(jobj,"refreshInterval",json_object_new_int(message->refresh_interval));


    int i = 0;
    struct json_object *jarray = json_object_new_array();
    for ( i = 0; i<message->value_count-1; i++) {

        struct json_object *jobji = json_object_new_object();

        json_object_object_add(jobji,"key",json_object_new_string(message->values[i].key));
        json_object_object_add(jobji,"type",json_object_new_string(message->values[i].type));
        json_object_object_add(jobji,"value",json_object_new_double(*(double*)message->values[i].value));

        json_object_array_add(jarray, jobji);
    }
    json_object_object_add(jobj,"values",jarray);

    // free the memory
    //free(message);

    return json_object_to_json_string(jobj);
}

