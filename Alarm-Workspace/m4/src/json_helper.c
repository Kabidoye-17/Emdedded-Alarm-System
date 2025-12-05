#include <string.h>
#include "command.h"
#include "json_helper.h"

int parse_command_event(const char* json_str, command_event* event) {
    const char* cmd_start = strstr(json_str, "\"commandValue\":\"") + 16; // pointer arithmetic to get to value 
    const char* ts_start = strstr(json_str, "\"timestamp\":\"") + 13;

    // Parse command based on first character pointed by cmd_start
    switch (*cmd_start) {
        case 'A': event->cmd = ARM; break;
        case 'D': event->cmd = DISARM; break;
        case 'R': event->cmd = RESOLVE; break;
        default: return -1;
    }

    // Copy timestamp (assuming fixed length of 24 characters)
    memcpy(event->timestamp, ts_start, 24);
    event->timestamp[24] = '\0';

    return 0;
}
