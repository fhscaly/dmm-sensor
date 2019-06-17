
#ifndef MESSAGE_PROTOCOL_H__
#define MESSAGE_PROTOCOL_H__

#define MAX_ARRAY_SIZE 24

#include <time.h>

/*
 *  key   -> name of the value field
 *  type  -> Type of the value field (SHORT, INTEGER, LONG, FLOAT, DOUBLE, BOOLEAN, TIMESTAMP, STRING, JSON)
 *  value -> Current (measurement) value as JSON string
 */
typedef struct {
    char *key;
    char *type;
    char *value;
} Message_value;

/*
 *  timestamp       -> timestamp of the event as UNIX Epoch milliseconds.
 *  warehouseId     -> Worldwide unique project number -> must be configurable!
 *  assetId         -> identification of the source (sensor, motor, component, etc.)
 *  assetName       -> Unique identification of the source (sensor, motor, component, etc.)
 *  eventType       -> Identifier of the message type.
 *  refreshInterval -> Time in seconds, when the next event is to be expected -> quasi a kind of watchdog Functionality. Set to -1 to deactivate.
 *  values          -> Array of [Key, Type, Value] triples with the current (measured) values. Maximum 24!

 */
typedef struct {
    time_t time;
    char *warehouse_id;
    int asset_id;
    char *asset_name;
    char *event_type;
    int refresh_interval;
    int value_count;
    Message_value* values;
} Message_header;

/*
 *  Create Header from message protocol
 *
 */
Message_header* create_message_header(char *warehouse_id, int asset_id, char *asset_name, char *event_type, int refresh_interval);

/*
 *  Adding value to message header
 *  Only support one value!!
 */
void add_value_to_message(Message_header *header, char *key, char *type, char *value);

#endif /* MESSAGE_PROTOCOL_H__*/