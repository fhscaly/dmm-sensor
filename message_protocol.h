

/*
 *  TODO: take description from MQTT message protocoll
 */
typedef struct {
    char *key;
    char *type;
    void *value;
} Message_value;

/*
 *  TODO: take description from MQTT message protocoll
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
 *  Adding value to message
 */
void add_value_to_message( Message_header *header, char * key, char * type, void* value);

