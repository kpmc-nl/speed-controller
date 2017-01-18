//
// Created by Matthijs Oosterhoff on 17/01/2017.
//

#include <wiring.h>
#include <core_pins.h>
#include <core_adc.h>
#include "controller.h"
#include "pinout.h"
#include "motor.h"
#include "rc_input.h"
#include "rc.h"
#include "settings.h"
#include "util.h"


#define LOOPTIME 1500


static uint64_t target_pulse = RC_PWM_NEUTRAL;
static int cutoff_voltage = 0;
static boolean battery_dead = false;

void controller_setup() {
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(FET_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    delay(500);

    int voltage = get_battery_voltage();
    uint8_t cell_count = 1 + (voltage / 4250);
    cutoff_voltage = 3400 * cell_count;

    motor_tone(600, 200);
    motor_tone(900, 200);
    motor_tone(1200, 200);

    if (rc_input_get_current() > RC_PWM_HIGH_THRESH) {
        /* run setup mode if controller is turned on while the signal is high */
        setup_mode();
        write_settins();
    } else {
        read_settings();
        delay(500);
    }

    validate_settings();
    wait_for_neutral();



    for (uint8_t i = 0; i < 2; i++) {
        delay(100);
        digitalWrite(LED1_PIN, HIGH);
        motor_tone(600, 200);
        digitalWrite(LED1_PIN, LOW);
    }

}

void controller_loop() {

    for (uint8_t i = 0; get_battery_voltage() < cutoff_voltage && i < 10; i++) {
        if (i == 9) {
            battery_dead = true;
        }
    }

     delayMicroseconds(LOOPTIME);
       uint64_t pulse = rc_input_get_current();

    if ((pulse <= RC_PWM_LOW_THRESH && target_pulse > RC_PWM_NEUTRAL) ||
        (pulse >= RC_PWM_HIGH_THRESH && target_pulse < RC_PWM_NEUTRAL)) {
        target_pulse = RC_PWM_NEUTRAL;
        return;
    } else if (pulse > target_pulse) {
        target_pulse++;
    } else if (pulse < target_pulse) {
        target_pulse--;
    }


    if (target_pulse > RC_PWM_LOW_THRESH && target_pulse < RC_PWM_HIGH_THRESH) {
        digitalWrite(FET_PIN, LOW);
        digitalWrite(LED2_PIN, HIGH);
        digitalWrite(LED1_PIN, LOW);
        return;
    }
    digitalWrite(LED2_PIN, LOW);

    if (target_pulse <= get_settings().min_pulse || target_pulse >= get_settings().max_pulse) {
        digitalWrite(LED1_PIN, HIGH);
    } else {
        digitalWrite(LED1_PIN, LOW);
    }

    uint8_t full_power = 255;
    if (battery_dead) {
        full_power = 64;
    }

    if (target_pulse >= RC_PWM_HIGH_THRESH) {
        motor_forward();
        motor_power((target_pulse - RC_PWM_HIGH_THRESH) * full_power / (get_settings().max_pulse - RC_PWM_HIGH_THRESH));
        return;
    }

    if (target_pulse <= RC_PWM_LOW_THRESH) {
        motor_reverse();
        motor_power((RC_PWM_LOW_THRESH - target_pulse) * full_power / (RC_PWM_LOW_THRESH - get_settings().min_pulse));
        return;
    }

}


void wait_for_neutral() {
    /* wait for neutral input */
    while (rc_input_get_current() > RC_PWM_HIGH_THRESH || rc_input_get_current() < RC_PWM_LOW_THRESH) {
        delay(10);
    }
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
}


int get_battery_voltage() {
    /* in millivolts */
    return map(analogRead(VOLTAGE_SENSOR), 0, 1023, 0, 21000);
}
