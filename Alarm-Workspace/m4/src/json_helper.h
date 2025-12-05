#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "command.h"

int parse_command_event(const char* json_str, command_event* event);

#endif // JSON_HELPER_H
