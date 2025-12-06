#ifndef COMMAND_H
#define COMMAND_H

typedef enum command_type {
    ARM,
    DISARM,
    RESOLVE,
    UNKNOWN_COMMAND
} command_type;

typedef struct {
    command_type cmd;
    char timestamp[25];
} command_event;

#endif // COMMAND_H