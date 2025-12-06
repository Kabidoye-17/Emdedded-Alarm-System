#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "command.h"


/**
 * Parses a JSON string containing a command event.
 * @param json_str The JSON string to parse (must not be NULL).
 *        Expected format: {"commandValue": "<command_name>", "timestamp": "<timestamp>"}
 * @param event Pointer to command_event struct to populate (must not be NULL).
 * @return 0 on success, -1 on error (malformed JSON or invalid command).
 */

int parse_command_event(const char* json_str, command_event* event);

#endif // JSON_HELPER_H
