
#include <Arduino.h> 

typedef enum { IDLE, MONITORING, WARNING, ERROR } SystemState;
typedef enum { EV_NONE, EV_START, EV_SENSOR_OK, EV_SENSOR_WARN, EV_SENSOR_ERR, EV_RESET } Event;

const int PIN_BUTTON_START = 2;
const int PIN_BUTTON_RESET = 3;
const int PIN_SENSOR = A0; 

void setup() {
    Serial.begin(9600);
    pinMode(PIN_BUTTON_START, INPUT_PULLUP); 
    pinMode(PIN_BUTTON_RESET, INPUT_PULLUP);
    Serial.println("Sistem pornit. Așteptare comenzi hardware...");
}

Event read_hardware_events() {
    if (digitalRead(PIN_BUTTON_START) == LOW) {
        delay(50); // Debounce simplu
        return EV_START;
    }
    if (digitalRead(PIN_BUTTON_RESET) == LOW) {
        delay(50);
        return EV_RESET;
    }
    
    // Citire senzor
    int val = analogRead(PIN_SENSOR);
    if (val > 700) return EV_SENSOR_ERR;
    if (val > 300) return EV_SENSOR_WARN;
    
    return EV_NONE;
}

SystemState handle_event(SystemState current, Event ev) {
    switch(current) {
        case IDLE:
            if (ev == EV_START) return MONITORING;
            break;
        case MONITORING:
            if (ev == EV_SENSOR_WARN) return WARNING;
            if (ev == EV_SENSOR_ERR) return ERROR;
            if (ev == EV_RESET) return IDLE;
            break;
        case WARNING:
            if (ev == EV_SENSOR_ERR) return ERROR;
            if (ev == EV_RESET) return IDLE;
            break;
        case ERROR:
            if (ev == EV_RESET) return IDLE;
            break;
    }
    return current;
}

void loop() {
    static SystemState state = IDLE; 

    Event ev = read_hardware_events();
    SystemState next_state = handle_event(state, ev);

    if (next_state != state) { 
        state = next_state;
        Serial.print("STATE NOU: ");
        if (state == IDLE) Serial.println("IDLE");
        else if (state == MONITORING) Serial.println("MONITORING");
        else if (state == WARNING) Serial.println("WARNING");
        else Serial.println("ERROR");
    }
    
    delay(100); 
}