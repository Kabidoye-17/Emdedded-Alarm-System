#include "state_machine.h"

static alarm_state transition(alarm_state state, alarm_event event) {
	switch (event) {
	case DISARM_SYSTEM:
		return DISARMED;
	default:
		break;
	}

	switch (state) {
	case DISARMED:
		if (event == ARM_SYSTEM)
			return ARMED_IDLE;
		break;
	case ARMED_IDLE:
		if (event == LOW_WARN)
			return WARN;
		if (event == MED_WARN)
			return ALERT;
		if (event == HIGH_WARN)
			return ALARM;
		break;
	case WARN:
		if (event == CANCEL_WARN)
			return ARMED_IDLE;
		if (event == MED_WARN)
			return ALERT;
		break;
	case ALERT:
		if (event == HIGH_WARN)
			return ALARM;
		if (event == RESOLVE_ALARM)
			return ARMED_IDLE;
		break;
	case ALARM:
		if (event == RESOLVE_ALARM)
			return ARMED_IDLE;
		break;
	default:
		break;
	}

	return state;
}

void alarm_sm_init(alarm_sm *sm) {
	if (!sm)
		return;
    // Default state is DISARMED
	sm->state = DISARMED;
}

void alarm_sm_handle_event(alarm_sm *sm, alarm_event event) {
	if (!sm)
		return;

	sm->state = transition(sm->state, event);
}

alarm_state alarm_sm_state(const alarm_sm *sm) {
	if (!sm)
		return DISARMED;

	return sm->state;
}

const char *alarm_state_name(alarm_state state) {
	switch (state) {
	case DISARMED:
		return "DISARMED";
	case ARMED_IDLE:
		return "ARMED_IDLE";
	case WARN:
		return "WARN";
	case ALERT:
		return "ALERT";
	case ALARM:
		return "ALARM";
	default:
		return "UNKNOWN";
	}
}
