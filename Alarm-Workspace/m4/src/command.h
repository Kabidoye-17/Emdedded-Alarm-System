#ifndef COMMAND_H
#define COMMAND_H

typedef enum command {
    ARM,
    DISARM,
    RESOLVE,
    UNKNOWN_COMMAND
} command;

typedef struct {
    command cmd;
    char timestamp[25];
} command_event;

#endif // COMMAND_H