#include <Arduino.h>
#include <Adafruit_MMA8451.h>
#include <time.h>
#include <music.h>

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

Adafruit_MMA8451 mma = Adafruit_MMA8451();

const uint8_t event_leds[]{
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

    pinMode(YELLOW_LED, OUTPUT);
    pinMode(WHITE_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    // Initialize MMA8451 accelerometer
    mma.begin();

    srand(time(0));
}

void loop() {
    uint8_t players = MIN_PLAYERS;
    while (true) {
        if (!digitalRead(DOWN_BUTTON) && players > MIN_PLAYERS) {
            players--;
            beep(SPEAKER, players, 200, 50);
            delay(250);
        }
        if (!digitalRead(UP_BUTTON) && players < MAX_PLAYERS) {
            players++;
            beep(SPEAKER, players, 200, 50);
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
        const unsigned long MILLIS_TILL_EVEMT = millis() + get_rand_int(5000, 15000);
        const uint8_t event = get_rand_int(0, 4);

        starting_note = play_music(SPEAKER, MILLIS_TILL_EVEMT, starting_note);

        // Begin event
        digitalWrite(event_leds[event], HIGH);

        const unsigned long start = millis();
        while (millis() < start + ROUND_DELAY_MILLIS) {
            // Wait until acceleration crosses below threshold before triggering alarm for going above.
            // This handles the circumstance in which the event is fired while the potato is mid-pass.
            // This way, the accelerometer threshold will not be checked until the next person has caught the potato
            // and/or it's decelerated enough to not trigger.
            while (get_acceleration() > ACCEL_THRESHOLD) {}

            if (get_acceleration() > ACCEL_THRESHOLD) {
                digitalWrite(RED_LED, HIGH);
                tone(SPEAKER, 440);
                delay(3000);
                digitalWrite(RED_LED, LOW);
                noTone(SPEAKER);
                break;
            }
        }
        digitalWrite(event_leds[event], LOW);

        players--;
    }
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