#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include "../utils/typing.h"

/*
 * Alarm system state machine.
 * Transitions occur through events such as arm/disarm (command_events) and warn levels (motion_events).
 */

// State change triggers
typedef enum alarm_event {
	EVENT_ARM_SYSTEM = 0,
	EVENT_DISARM_SYSTEM,
	EVENT_LOW_WARN,
	EVENT_MED_WARN,
	EVENT_HIGH_WARN,
	EVENT_RESOLVE_ALARM,
	EVENT_CANCEL_WARN // Produced when low warn timeout occurs
} alarm_event;

// State machine structure (Just a container for current state)
typedef struct alarm_sm {
	alarm_state state;
} alarm_sm;

// Set state to default state (DISARMED)
void alarm_sm_init(alarm_sm *sm);

// Handle alarm event and perform state transition if necessary
void alarm_sm_handle_event(alarm_sm *sm, alarm_event event);

// Get current state of the state machine
alarm_state alarm_sm_state(const alarm_sm *sm);

/// Get string name of the given state
const char *alarm_state_name(alarm_state state);

#endif /* STATE_MACHINE_H */
