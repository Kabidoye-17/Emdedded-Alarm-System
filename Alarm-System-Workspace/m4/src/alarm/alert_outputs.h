#ifndef ALERT_OUTPUTS_H
#define ALERT_OUTPUTS_H

typedef enum {
    OFF = 0,
    BLUE,
    GREEN,
    RED,
    RED_BREATHE,
    RED_FLASH
} LedMode;

void alert_outputs_set_mode(LedMode mode);
void alert_outputs_init(void);

#endif /* ALERT_OUTPUTS_H */