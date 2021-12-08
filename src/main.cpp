#include <Arduino.h>
#include <Adafruit_MMA8451.h>
#include <time.h>
#include <music.h>
#include <ArduinoSTL.h>

#define VIB_MOTOR_1 A1
#define VIB_MOTOR_2 A3
#define VIB_MOTOR_3 5
#define VIB_MOTOR_4 4
#define VIB_MOTOR_5 3
#define VIB_MOTOR_6 1

#define DOWN_BUTTON 13
#define START_BUTTON 12
#define UP_BUTTON 11

#define SPEAKER A0

#define YELLOW_LED 10
#define WHITE_LED 9
#define BLUE_LED 8
#define GREEN_LED 6

#define MIN_PLAYERS 2
#define MAX_PLAYERS 10
#define ROUND_DELAY_MILLIS 5000
#define ACCEL_THRESHOLD 30

void play_game(uint8_t players);

double get_acceleration();

void beep(uint8_t pin, uint8_t repetitions, long duration, long rest);

void toggle_motors(bool state);

void toggle_leds(bool state);

std::vector<uint8_t> get_potential_events(uint8_t PLAYERS, uint8_t STARTING_PLAYERS);

Adafruit_MMA8451 mma = Adafruit_MMA8451();

const uint8_t VIB_MOTOR_PINS[]{
        VIB_MOTOR_1,
        VIB_MOTOR_2,
        VIB_MOTOR_3,
        VIB_MOTOR_4,
        VIB_MOTOR_5,
        VIB_MOTOR_6
};

const uint8_t LED_PINS[]{
        YELLOW_LED,
        WHITE_LED,
        BLUE_LED,
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
    if (!mma.begin()) {
        tone(SPEAKER, 440);
    }

    srand(time(0));
}

void loop() {
    uint8_t players = MIN_PLAYERS;

    while (true) {
        if (!digitalRead(DOWN_BUTTON) && players > MIN_PLAYERS) {
            players--;
            beep(SPEAKER, players, 100, 50);
            delay(250);
        }
        if (!digitalRead(UP_BUTTON) && players < MAX_PLAYERS) {
            players++;
            beep(SPEAKER, players, 100, 50);
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
    const uint8_t STARTING_PLAYERS = players;

    while (players > 1) {

        const uint16_t tempo = (players < 7) ? 240 - 20 * players : 100;

        // Play music starting at note index starting_note for a random amount of time between 3 and 5 seconds.
        // play_music() blocks execution until the given duration has elapsed, serving as the timer till the event.
        starting_note = play_music(SPEAKER, random(5000, 10000), starting_note, tempo);

        // If starting_note == 0, song/round is over
        if (starting_note == 0) {
            // Turn on all LEDs and vibrate at end of round
            toggle_leds(true);
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
                    toggle_motors(false);
                    beep(SPEAKER, 1, 3000, 0);
                }
            }

            // Stop LEDs and vibration to begin next round
            toggle_leds(false);
            toggle_motors(false);

            players--;
        } else {
            // Get vector of all possible events given current and initial number of players
            const std::vector<uint8_t> potential_events = get_potential_events(players, STARTING_PLAYERS);

            // Ensure LED from previous event is off before starting event
            toggle_leds(false);

            // Begin random event if an event is possible
            if (!potential_events.empty()) {
                digitalWrite(potential_events[random(0, potential_events.size())], HIGH);

                if (digitalRead(BLUE_LED)) {
                    players++;
                }
            }
        }
    }
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
    for (const uint8_t PIN: VIB_MOTOR_PINS) {
        digitalWrite(PIN, state);
    }
}

void toggle_leds(const bool state) {
    for (const uint8_t pin: LED_PINS) {
        digitalWrite(pin, state);
    }
}

// Exclude previous event from occurring again
void conditional_add(std::vector<uint8_t> &VEC, const uint8_t LED) {
    if (!digitalRead(LED)) {
        VEC.push_back(LED);
    }
}

std::vector<uint8_t> get_potential_events(const uint8_t PLAYERS, const uint8_t STARTING_PLAYERS) {
    std::vector<uint8_t> vec;

    conditional_add(vec, YELLOW_LED);
    conditional_add(vec, GREEN_LED);

    if (PLAYERS > 3) {
        conditional_add(vec, WHITE_LED);
    }

    // Reduce probability of resurrection
    if (PLAYERS < STARTING_PLAYERS && random(0, 4) == 0) {
        conditional_add(vec, BLUE_LED);
    }

    return vec;
}
