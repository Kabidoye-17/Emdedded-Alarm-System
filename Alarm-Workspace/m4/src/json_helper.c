#include <string.h>
#include "command.h"
#include "json_helper.h"

#define COMMAND_VALUE_KEY "\"commandValue\":\""
#define TIMESTAMP_KEY "\"timestamp\":\""

#define TIMESTAMP_LENGTH 24

int parse_command_event(const char* json_str, command_event* event) {
    if (!json_str || !event) {
        return -1;
    }

    const char* cmd_start = strstr(json_str, COMMAND_VALUE_KEY);
    if (!cmd_start) return -1;
    cmd_start += strlen(COMMAND_VALUE_KEY);

    const char* ts_start = strstr(json_str, TIMESTAMP_KEY);
    if (!ts_start) return -1;
    ts_start += strlen(TIMESTAMP_KEY);

    // Parse command based on first character pointed by cmd_start
    switch (*cmd_start) {
        case 'A': event->cmd = ARM; break;
        case 'D': event->cmd = DISARM; break;
        case 'R': event->cmd = RESOLVE; break;
        default: return -1;
    }

    // Find end of timestamp value and validate length
    const char* ts_end = strchr(ts_start, '"');
    if (!ts_end || (ts_end - ts_start) != TIMESTAMP_LENGTH) {
        return -1;
    }

    // Copy timestamp
    memcpy(event->timestamp, ts_start, TIMESTAMP_LENGTH);
    event->timestamp[TIMESTAMP_LENGTH] = '\0';

    return 0;
}
