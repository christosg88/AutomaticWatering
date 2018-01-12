////////////////////////////////// LIBRARIES ///////////////////////////////////
// Used to create non blocking simultaneous code modules
#include <SimpleTimer.h>
// Used to de-bounce the button
#include <Bounce2.h>

/////////////////////// PROTOTYPES AND GLOBAL VARIABLES ////////////////////////
SimpleTimer timer;

void checkSoilHumidity();
    bool watering = false;
    int wateringTimerID = timer.MAX_TIMERS;
    int humidityPerCent();
    void togglePumpState();
        bool pumpState = LOW;


void checkReservoirLevel();
    bool reservoirEmpty = false;
    void notifyEmptyReservoir();

void checkPumpOverride();
    Bounce bouncer = Bounce();

void log();
    unsigned int cur_min = 1023;
    unsigned int cur_max = 0;

////////////////////////////////// CONSTANTS ///////////////////////////////////
const int pumpPin = 2;
const int reservoirPin = 3;
const int buzzerPin = 4;
const int buttonPin = 5;
const int humidityPin = A0;
// From experimental measurements, a value of around 200 means well watered
// while a value of around 600 means not watered at all
const int MIN_HUMIDITY = 200;
const int MAX_HUMIDITY = 600;

////////////////////////////// BUILTIN FUNCTIONS ///////////////////////////////
void setup() {
    Serial.begin(9600);

    pinMode(pumpPin, OUTPUT);
    pinMode(reservoirPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(buttonPin, INPUT);
    bouncer.attach(buttonPin);
    bouncer.interval(5);

    timer.setInterval(5000, checkSoilHumidity);
    timer.setInterval(5000, checkReservoirLevel);
    timer.setInterval(0, checkPumpOverride);
    timer.setInterval(1000, log);
}

void loop() {
    timer.run();
}

/////////////////////////////// IMPLEMENTATIONS ////////////////////////////////
/**
 * Check the soil's humidity, and if it falls bellow 10%, activate the pump.
 * Stop the pump once the humidity goes above 50%.
 */
void checkSoilHumidity() {
    if (humidityPerCent() < 10) {
        watering = true;
    }
    else if (humidityPerCent() > 50) {
        watering = false;
    }

    if (watering) {
        // Activate the pump for one second and then deactivate it. We activate
        // it in bursts so that the soil has time to absorb the water and the
        // sensor can pick up the new humidity.
        timer.setTimeout(0, togglePumpState);
        timer.setTimeout(1000, togglePumpState);
    }
}

/**
 * Return the soil humidity, in percentage.
 * @return The humidity in percentage. 100% means most humid, and 0% not humid
 *             at all
 */
int humidityPerCent() {
    // Read the sensor's value
    int humidityValue = analogRead(humidityPin);

    humidityValue = constrain(humidityValue, MIN_HUMIDITY, MAX_HUMIDITY);
    humidityValue = map(humidityValue, MIN_HUMIDITY, MAX_HUMIDITY, 100, 0);

    return humidityValue;
}

/**
 * Toggle the pump's state to activate or deactivate it.
 */
void togglePumpState() {
    pumpState = !pumpState;
    digitalWrite(pumpPin, pumpState);
    digitalWrite(LED_BUILTIN, pumpState);
}

/**
 * Check if the reservoir is empty, and if it is set the homonymous variable and
 * notify using the buzzer.
 */
void checkReservoirLevel() {
    if (!digitalRead(reservoirPin)) {
        reservoirEmpty = true;

        timer.setTimeout(0, notifyEmptyReservoir);
        timer.setTimeout(400, notifyEmptyReservoir);
    }
    else {
        reservoirEmpty = false;
    }
}

/**
 * Activate the buzzer for 200 ms at 2500 Hz.
 */
void notifyEmptyReservoir() {
    tone(buzzerPin, 3729, 200);
}

/**
 * While the button is being pressed, and the reservoir is not empty, keep the
 * pump running. Used to empty the reservoir.
 */
void checkPumpOverride() {
    bouncer.update();
    if (bouncer.rose()) {
        togglePumpState();
    }
}

/**
 * Just for debugging purposes.
 */
void log() {
    // Get the current value of hygrometer's sensor
    unsigned int cur = analogRead(humidityPin);
    // Update minimum and maximum measured values
    if (cur < cur_min)
        cur_min = cur;
    else if (cur > cur_max)
        cur_max = cur;

    // Print values as: current (minimum, maximum)
    Serial.print(cur);
    Serial.print(" (");
    Serial.print(cur_min);
    Serial.print(", ");
    Serial.print(cur_max);
    Serial.println(")");

    // Print current value in percentage
    Serial.print("hygro: ");
    Serial.print(humidityPerCent());
    Serial.println("%");
    Serial.println();
}
