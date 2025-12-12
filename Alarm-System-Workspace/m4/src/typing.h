#ifndef TYPING_H
#define TYPING_H

#define MAX_TIMESTAMP_LENGTH 20

// ===================== ENUMS =====================

// -> received from motion sensor
typedef enum warn_type {
    LOW_WARN,
    MED_WARN,
    HIGH_WARN,
} warn_type;

// -> received from UART ISR callback
typedef enum command_type {
    ARM,
    DISARM,
    RESOLVE_ALARM,
} command_type;

// -> actual states of the alarm system
typedef enum alarm_state {
	DISARMED = 0,
	ARMED_IDLE,
	WARN,
	ALERT,
	ALARM,
} alarm_state;

// ===================== STRUCTS =====================

// -> motion_queue contents
typedef struct motion_event {
    enum warn_type warning;
    char timestamp[MAX_TIMESTAMP_LENGTH];
} motion_event;

// -> command_queue contents
typedef struct command_event {
    enum command_type cmd;
} command_event;

// -> cloud_queue contents
typedef struct cloud_update_event {
    uint8_t from_motion : 1; // boolean bitfield
    enum warn_type warning; // null if !from_motion
    enum alarm_state state;
    char timestamp[MAX_TIMESTAMP_LENGTH]; // null if !from_motion
} cloud_update_event; 

#endif /* TYPING_H */