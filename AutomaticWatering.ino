#include "Bounce2.h"

// From experimental measurements, a value of around 200 means well watered
// while a value of around 600 means not watered at all
const int MIN = 200;
const int MAX = 600;

const int pumpPin = 2;
const int waterLevelPin = 3;
const int buzzerPin = 4;
const int buttonPin = 5;
const int hygroPin = A0;

bool wattering = false;

// Button (debounced)
Bounce bouncer = Bounce();

// Prototypes
void log();
int hygroPerCent();

void setup() {
    Serial.begin(9600);
    pinMode(pumpPin, OUTPUT);
    pinMode(waterLevelPin, INPUT);
    pinMode(buzzerPin, OUTPUT);

    pinMode(buttonPin, INPUT);
    bouncer.attach(buttonPin);
    bouncer.interval(5);
}

void loop() {
    log();

    /**
     * If the soil humidity falls bellow 10%, activate the pump for one second,
     * and deactivate it. Do it again on the next loop, until the humidity goes
     * above 70%
     */
    if (hygroPerCent() < 10)
        wattering = true;
    else if (hygroPerCent() > 70)
        wattering = false;

    if (wattering) {
        // Activate the pump in bursts, so that the sensor has time to respond
        digitalWrite(pumpPin, HIGH);
        delay(1000);
        digitalWrite(pumpPin, LOW);
        delay(4000);
    }

    /**
     * If reservoir is empty, activate buzzer
     */
    if (!digitalRead(waterLevelPin)) {
        tone(buzzerPin, 2500, 200);
        delay(300);
        tone(buzzerPin, 2500, 200);
    }

    /**
     * While the button is being pressed, and the reservoir is not empty, keep
     * the pump running
     */
    bouncer.update();
    if (bouncer.rose()) {
        digitalWrite(pumpPin, HIGH);
    }
    else if (bouncer.fell()) {
        digitalWrite(pumpPin, LOW);
    }

    delay(2000);
}

/**
 * Just for debugging purposes.
 */
unsigned int min = 1023;
unsigned int max = 0;
void log() {
    // Get the current value of hygrometer's sensor
    unsigned int cur = analogRead(hygroPin);
    // Update minimum and maximum measured values
    if (min > cur)
        min = cur;
    if (max < cur)
        max = cur;

    // Print values as: current (minimum, maximum)
    Serial.print(cur);
    Serial.print(" (");
    Serial.print(min);
    Serial.print(", ");
    Serial.print(max);
    Serial.println(")");

    // Print current value in percentage
    Serial.print("hygro: ");
    Serial.print(hygroPerCent());
    Serial.println("%");
    Serial.println();
}

/**
 * Return the soil humidity, in percentage
 * @return          The humidity in percentage. 100% means most humid, and 0%
 *                  not humid at all
 */
int hygroPerCent() {
    // Read the sensor's value
    int hygroValue = analogRead(hygroPin);

    hygroValue = constrain(hygroValue, MIN, MAX);
    hygroValue = map(hygroValue, MIN, MAX, 100, 0);

    return hygroValue;
}
