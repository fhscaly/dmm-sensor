
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "message_protocol.h"



void add_value_to_message(Message_header *header, char *key, char *type, char *value)
{
    /* TODO
     * may reallocate memory of header->values to support more than one value in values
     * For now only one value are necessary
     */
    header->value_count++;

    // pointer arithmetic - select the value to add
    Message_value *mv = header->values + header->value_count -1;
    mv->key = key;
    mv->type = type;
    mv->value = value;
}

Message_header* create_message_header(char *warehouse_id, int asset_id, char *asset_name, char *event_type, int refresh_interval)
{
    Message_header *mh = malloc(sizeof(Message_header));

    mh->time = time(NULL);
    mh->warehouse_id = warehouse_id;
    mh->asset_id = asset_id;
    mh->asset_name = asset_name;
    mh->event_type = event_type;
    mh->refresh_interval = refresh_interval;
    mh->value_count = 0;
    mh->values = malloc(sizeof(Message_value));

    return mh;
}
