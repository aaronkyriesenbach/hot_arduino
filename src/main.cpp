#include <Arduino.h>
#include <Adafruit_MMA8451.h>
#include <time.h>

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

double get_acceleration();

int get_rand_int(int min, int max);

void play_game(uint8_t players);

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
            Serial.print("Players: ");
            Serial.println(players);
            delay(250);
        }
        if (!digitalRead(UP_BUTTON) && players < MAX_PLAYERS) {
            players++;
            Serial.print("Players: ");
            Serial.println(players);
            delay(250);
        }
        if (!digitalRead(START_BUTTON)) {
            Serial.println("Starting game");
            play_game(players);
            break;
        }
    }
}

void play_game(uint8_t players) {
    while (players > 1) {
        // Time between 5 and 15 seconds in the future for an event to fire
        uint64_t event_millis = millis() + get_rand_int(5000, 15000);
        uint8_t event = get_rand_int(0, 4);

        while (millis() < event_millis) {
            // play_music();
        }

        // Begin event
        digitalWrite(event_leds[event], HIGH);

        uint64_t start = millis();
        while (millis() < start + ROUND_DELAY_MILLIS) {
            if (get_acceleration() > 20.0) {
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