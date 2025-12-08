#include "state_machine.h"

static enum alarm_state transition(enum alarm_state state, enum alarm_event event) {
	switch (event) {
	case ALARM_EVENT_DISARM_SYSTEM:
		return ALARM_STATE_DISARMED;
	default:
		break;
	}

	switch (state) {
	case ALARM_STATE_DISARMED:
		if (event == ALARM_EVENT_ARM_SYSTEM)
			return ALARM_STATE_ARMED_IDLE;
		break;
	case ALARM_STATE_ARMED_IDLE:
		if (event == ALARM_EVENT_LOW_WARN)
			return ALARM_STATE_WARN;
		if (event == ALARM_EVENT_MED_WARN)
			return ALARM_STATE_ALERT;
		if (event == ALARM_EVENT_HIGH_WARN)
			return ALARM_STATE_ALARM;
		break;
	case ALARM_STATE_WARN:
		if (event == ALARM_EVENT_CANCEL_WARN)
			return ALARM_STATE_ARMED_IDLE;
		if (event == ALARM_EVENT_MED_WARN)
			return ALARM_STATE_ALERT;
		break;
	case ALARM_STATE_ALERT:
		if (event == ALARM_EVENT_HIGH_WARN)
			return ALARM_STATE_ALARM;
		if (event == ALARM_EVENT_RESOLVE_ALARM)
			return ALARM_STATE_ARMED_IDLE;
		break;
	case ALARM_STATE_ALARM:
		if (event == ALARM_EVENT_RESOLVE_ALARM)
			return ALARM_STATE_ARMED_IDLE;
		break;
	default:
		break;
	}

	return state;
}

void alarm_sm_init(struct alarm_sm *sm) {
	if (!sm)
		return;
    // Default state is DISARMED
	sm->state = ALARM_STATE_DISARMED;
}

void alarm_sm_handle_event(struct alarm_sm *sm, enum alarm_event event) {
	if (!sm)
		return;

	sm->state = transition(sm->state, event);
}

enum alarm_state alarm_sm_state(const struct alarm_sm *sm) {
	if (!sm)
		return ALARM_STATE_DISARMED;

	return sm->state;
}

const char *alarm_state_name(enum alarm_state state) {
	switch (state) {
	case ALARM_STATE_DISARMED:
		return "DISARMED";
	case ALARM_STATE_ARMED_IDLE:
		return "ARMED_IDLE";
	case ALARM_STATE_WARN:
		return "WARN";
	case ALARM_STATE_ALERT:
		return "ALERT";
	case ALARM_STATE_ALARM:
		return "ALARM";
	default:
		return "UNKNOWN";
	}
}
