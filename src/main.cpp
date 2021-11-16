//void loop() {
//    int num_players = 2;
//    if (digitalRead(UP_BUTTON)) { num_players++; }
//    else if (digitalRead(DOWN_BUTTON)) { num_players--; }
//    if (digitalRead(START_BUTTON)) { play_game(num_players); }
//}
//
//
//void play_game(int num_players) {
//    int millis_to_next_event = rand() * 10 + 10;
//    while (num_players > 1) {
//        play_music();
//        if (millis_to_next_event == millis()) {
//            events[rand() * 4](&num_players);
//            millis_to_next_event = 0;
//        }
//    }
//}

#include <Arduino.h>
#include <Adafruit_MMA8451.h>

double get_acceleration();

Adafruit_MMA8451 mma = Adafruit_MMA8451();


void setup() {
    Serial.begin(9600);

    // Initialize MMA8451 accelerometer
    mma.begin();
}

void loop() {
    Serial.print("Acceleration: ");
    Serial.println(get_acceleration());
}

// Returns a double representing the overall acceleration of the MMA8451 in m/s^2. Note that at rest the acceleration
// will be ~9.81 m/s^2 due to gravity.
double get_acceleration() {
    // Retrieve a new accelerometer event
    sensors_event_t event;
    mma.getEvent(&event);

    return sqrt(pow(event.acceleration.x, 2) + pow(event.acceleration.y, 2) + pow(event.acceleration.z, 2))
}