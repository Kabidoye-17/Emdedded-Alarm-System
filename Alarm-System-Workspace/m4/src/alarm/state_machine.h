#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>

/*
 * Alarm system state machine.
 * Transitions occur through events such as arm/disarm and warn levels.
 */

// Actual states of the alarm system
enum alarm_state {
	ALARM_STATE_DISARMED = 0,
	ALARM_STATE_ARMED_IDLE,
	ALARM_STATE_WARN,
	ALARM_STATE_ALERT,
	ALARM_STATE_ALARM,
};

// State change triggers
enum alarm_event {
	ALARM_EVENT_ARM_SYSTEM = 0,
	ALARM_EVENT_DISARM_SYSTEM,
	ALARM_EVENT_LOW_WARN,
	ALARM_EVENT_MED_WARN,
	ALARM_EVENT_HIGH_WARN,
	ALARM_EVENT_RESOLVE_ALARM,
	ALARM_EVENT_CANCEL_WARN, // Produced when timeouts occurs
};

// State machine structure (Just a container for current state)
struct alarm_sm {
	enum alarm_state state;
};

// Set state to default state (ALARM_STATE_DISARMED)
void alarm_sm_init(struct alarm_sm *sm);

// Handle alarm event and perform state transition if necessary
void alarm_sm_handle_event(struct alarm_sm *sm, enum alarm_event event);

// Get current state of the state machine
enum alarm_state alarm_sm_state(const struct alarm_sm *sm);

/// Get string name of the given state
const char *alarm_state_name(enum alarm_state state);

#endif /* STATE_MACHINE_H */
