//
// Created by Matthijs Oosterhoff on 25/05/16.
//

#include <Arduino.h>
#include "Settings.h"
#include "RCInput.h"
#include "Controller.h"

void Settings::setupMode() {
    /* inidcate that we are running the setup mode */

    for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(LED1_PIN, HIGH);
        digitalWrite(LED2_PIN, HIGH);
        delay(100);
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
        delay(100);
    }

    /* read max pulse, assuming the user gives max input */
    data.maxPulse = getCurrentRcInput() - RC_PWM_OUTER_THRESH;
    digitalWrite(LED1_PIN, HIGH);
    motorTone(1200, 300);

    /* wait for low pulse */
    while (getCurrentRcInput() > RC_PWM_LOW_THRESH) {
        delay(10);
    }
    delay(500);
    /* read low pulse, assuming the user gives min input */
    data.minPulse = getCurrentRcInput() + RC_PWM_OUTER_THRESH;
    digitalWrite(LED2_PIN, HIGH);
    motorTone(1200, 300);

}

void Settings::readSettings() {
    /* read settings from eeprom if controller is started while the signal is neutral or low */
    eeprom_read_block(&data, &eeData, sizeof(SettingsData));

}

void Settings::writeSettings() {
    /* store the settings in the eeprom */
    eeprom_write_block(&data, &eeData, sizeof(SettingsData));
}


void Settings::validateSettings() {
    // Validate settings as much as we can. If any error is detected,
    // the leds will blink furiously, to indicate that the setup must be performed.
    if (data.minPulse < 500 || data.minPulse > RC_PWM_LOW_THRESH ||
        data.maxPulse > 2500 || data.maxPulse < RC_PWM_HIGH_THRESH ||
        data.minPulse > data.maxPulse) {
        while (getCurrentRcInput() < RC_PWM_HIGH_THRESH) {

            digitalWrite(LED1_PIN, HIGH);
            digitalWrite(LED2_PIN, LOW);
            motorTone(900, 100);
            delay(100);
            digitalWrite(LED2_PIN, HIGH);
            digitalWrite(LED1_PIN, LOW);
            motorTone(900, 100);
            delay(100);
        }
        setupMode();
        writeSettings();
    }
}

void Settings::init() {
    delay(500);

    motorTone(600, 200);
    motorTone(900, 200);
    motorTone(1200, 200);

    if (getCurrentRcInput() > RC_PWM_HIGH_THRESH) {
        /* run setup mode if controller is turned on while the signal is high */
        setupMode();
        writeSettings();
    } else {
        readSettings();
        delay(500);
    }

    validateSettings();


    motorTone(900, 300);
    delay(100);
    motorTone(900, 300);
}

uint64_t Settings::getMinPulse() {
    return data.minPulse;
}

uint64_t Settings::getMaxPulse() {
    return data.maxPulse;
}

void Settings::motorTone(uint64_t freq, uint64_t duration) {
    uint64_t period_usec = 1000000 / freq;
    uint64_t k = duration * 1000 / period_usec;

    for (uint64_t i = 0; i < k; i++) {
        digitalWrite(FET_PIN, HIGH);
        delayMicroseconds(1);
        digitalWrite(FET_PIN, LOW);
        delayMicroseconds(period_usec);
    }
}