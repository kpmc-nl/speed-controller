//
// Created by Matthijs Oosterhoff on 17/01/2017.
//

#include "motor.h"
#include <Arduino.h>
#include "pinout.h"

static boolean reverse;

void motor_setup() {
    pinMode(FORWARD_PIN, OUTPUT);
    pinMode(REVERSE_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    /* Set WGM (Wave Form Generation Mode) to Fast PWM */
    TCCR0A |= (1 << WGM00) | (1 << WGM01);

    /* Set CS (Clock Select) to internal clock 0; IE no prescaling */
      TCCR0B |= ( 1 << CS00);                   //    0 0 1 clk I/O /(No prescaling)
//    TCCR0B |= ( 1 << CS01 );                  //    0 1 0 clk I/O /8 (From prescaler)
//    TCCR0B |= ( 1 << CS01 ) | ( 1<< CS00 );   //    0 1 1 clk I/O /64 (From prescaler)
//    TCCR0B |= ( 1 << CS02 );                  //    1 0 0 clk I/O /256 (From prescaler)
//    TCCR0B |= ( 1 << CS02 ) | ( 1<< CS00 );   //    1 0 1 clk I/O /1024 (From prescaler)
}

void motor_power(int power) {

    if (reverse) {
        digitalWrite(FORWARD_PIN, 0);
        digitalWrite(REVERSE_PIN, 1);
        analogWrite(ENABLE_PIN, power);
    } else {
        digitalWrite(FORWARD_PIN, 1);
        digitalWrite(REVERSE_PIN, 0);
        analogWrite(ENABLE_PIN, power);
    }
}

void motor_stop() {

    digitalWrite(FORWARD_PIN, 0);
    digitalWrite(REVERSE_PIN, 0);
    digitalWrite(ENABLE_PIN, 1);
}

void motor_reverse() {
    reverse = true;
}

void motor_forward() {
    reverse = false;
}

void motor_tone(uint64_t freq, uint64_t duration) {

    uint64_t period_usec = 1000000 / freq;
    uint64_t k = duration * 1000 / period_usec;

    digitalWrite(FORWARD_PIN, HIGH);
    digitalWrite(REVERSE_PIN, LOW);
    for (uint64_t i = 0; i < k; i++) {
        digitalWrite(ENABLE_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(ENABLE_PIN, LOW);
        delayMicroseconds(period_usec);
    }
    digitalWrite(FORWARD_PIN, LOW);
    digitalWrite(REVERSE_PIN, LOW);

}
