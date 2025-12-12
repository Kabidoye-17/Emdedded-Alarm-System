#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include "typing.h"

/*
 * Alarm system state machine.
 * Transitions occur through events such as arm/disarm and warn levels.
 */

// State change triggers
typedef enum alarm_event {
	ARM_SYSTEM = 0,
	DISARM_SYSTEM,
	LOW_WARN,
	MED_WARN,
	HIGH_WARN,
	RESOLVE_ALARM,
	CANCEL_WARN, // Produced when timeouts occurs
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
