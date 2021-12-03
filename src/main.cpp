#include <Arduino.h>
#include <Adafruit_MMA8451.h>
#include <time.h>
#include <music.h>

#define VIB_MOTOR_1 2

#define DOWN_BUTTON 4
#define START_BUTTON 5
#define UP_BUTTON 6

#define SPEAKER 7

#define YELLOW_LED 8
#define WHITE_LED 9
#define BLUE_LED 10
#define RED_LED 11
#define GREEN_LED 12

#define MIN_PLAYERS 2
#define MAX_PLAYERS 10
#define ROUND_DELAY_MILLIS 5000
#define ACCEL_THRESHOLD 20.0

void play_game(uint8_t players);

double get_acceleration();

int get_rand_int(int min, int max);

void beep(uint8_t pin, uint8_t repetitions, long duration, long rest);

void toggle_motors(bool state);

Adafruit_MMA8451 mma = Adafruit_MMA8451();

const uint8_t VIB_MOTOR_PINS[]{
        VIB_MOTOR_1
};

const uint8_t LED_PINS[]{
        YELLOW_LED,
        WHITE_LED,
        BLUE_LED,
        RED_LED,
        GREEN_LED
};

void setup() {
    Serial.begin(9600);

    pinMode(DOWN_BUTTON, INPUT_PULLUP);
    pinMode(START_BUTTON, INPUT_PULLUP);
    pinMode(UP_BUTTON, INPUT_PULLUP);

    for (const uint8_t pin: VIB_MOTOR_PINS) {
        pinMode(pin, OUTPUT);
    }

    for (const uint8_t pin: LED_PINS) {
        pinMode(pin, OUTPUT);
    }

    // Initialize MMA8451 accelerometer
    mma.begin();

    srand(time(0));
}

void loop() {
    uint8_t players = MIN_PLAYERS;
    while (true) {
        if (!digitalRead(DOWN_BUTTON) && players > MIN_PLAYERS) {
            players--;
            beep(SPEAKER, players, 100, 25);
            delay(250);
        }
        if (!digitalRead(UP_BUTTON) && players < MAX_PLAYERS) {
            players++;
            beep(SPEAKER, players, 100, 25);
            delay(250);
        }
        if (!digitalRead(START_BUTTON)) {
            play_game(players);
            break;
        }
    }
}

void play_game(uint8_t players) {
    uint16_t starting_note = 0;

    while (players > 1) {
        // Time between 5 and 15 seconds in the future for an event to fire
        const unsigned long EVENT_MILLIS = millis() + get_rand_int(5000, 15000);
        const uint8_t EVENT = get_rand_int(0, 4);

        // play_music() blocks execution until the event should fire, serving as the timer to the event.
        starting_note = play_music(SPEAKER, EVENT_MILLIS - millis(), starting_note);

        // If starting_note == 0, song/round is over
        if (starting_note == 0) {
            // Turn on all LEDs, vibrate
            for (const uint8_t PIN: LED_PINS) {
                digitalWrite(PIN, HIGH);
            }

            toggle_motors(true);

            // Yell at player if potato is passed after round ends
            const unsigned long NEXT_ROUND_START = millis() + ROUND_DELAY_MILLIS;
            while (millis() < NEXT_ROUND_START) {
                // Wait until acceleration crosses below threshold before triggering alarm for going above.
                // This handles the circumstance in which the round ends while the potato is mid-pass.
                // This way, the accelerometer threshold will not be checked until the next person has caught the potato
                // and/or it's decelerated enough to not trigger.
                while (get_acceleration() > ACCEL_THRESHOLD) {}

                if (get_acceleration() > ACCEL_THRESHOLD) {
                    beep(SPEAKER, 1, 3000, 0);
                    break;
                }
            }

            for (const uint8_t led: LED_PINS) {
                digitalWrite(led, LOW);
            }

            toggle_motors(false);

            players--;
        } else {
            // Begin event
            digitalWrite(LED_PINS[EVENT], HIGH);
            delay(ROUND_DELAY_MILLIS);
            digitalWrite(LED_PINS[EVENT], LOW);
        }
    }

    toggle_motors(true);
    delay(5000);
    toggle_motors(false);
}

int get_rand_int(const int min, const int max) {
    return rand() % (max - min + 1) + min;
}

// Returns a double representing the overall acceleration of the MMA8451 in m/s^2. Note that at rest the acceleration
// will be ~9.81 m/s^2 due to gravity.
double get_acceleration() {
    // Retrieve a new accelerometer event
    sensors_event_t event;
    mma.getEvent(&event);

    return sqrt(pow(event.acceleration.x, 2) + pow(event.acceleration.y, 2) + pow(event.acceleration.z, 2));
}

void beep(uint8_t pin, uint8_t repetitions, long duration, long rest) {
    for (int i = 0; i < repetitions; i++) {
        tone(pin, 440);
        delay(duration);
        noTone(pin);
        delay(rest);
    }
}

void toggle_motors(const bool state) {
    for (const uint8_t PIN : VIB_MOTOR_PINS) {
        digitalWrite(PIN, state);
    }
}