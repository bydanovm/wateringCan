#include "relayIn.h"

#if defined(ESP8266)
    #define RECIEVE_ATTR ICACHE_RAM_ATTR
#else
    #define RECIEVE_ATTR
#endif

RelayIn::RelayIn(){}
RelayIn::RelayIn(byte _pin, byte _mode){
    pin = _pin;
    mode = _mode;
    pinMode(pin, mode);
}
bool RelayIn::getCondition(void){
    return condition;
}

byte RelayIn::getErrorStatus(void){
    return errorStatus;
}
void RelayIn::clearErrorStatus(void){
    errorStatus = 0x00;
}

// Включение прерывания
void RelayIn::onInt(){
    if(((mode & INPUT) == INPUT) || ((mode & INPUT_PULLUP) == INPUT_PULLUP)){
        switch(pin){
            case buttonPinStart:
                #if defined(ESP8266)
                    attachInterrupt(inRelay1Pin, inRelay1Int, CHANGE);
                #else
                    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay1Int, CHANGE);
                #endif
                instances[0] = this;
                break;
            case buttonPinStop:
                #if defined(ESP8266)
                    attachInterrupt(inRelay2Pin, inRelay2Int, CHANGE);
                #else
                    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay2Int, CHANGE);
                #endif
                instances[1] = this;
                break;
            // case inRelay3Pin:
            //     #if defined(ESP8266)
            //         attachInterrupt(inRelay3Pin, inRelay3Int, CHANGE);
            //     #else
            //         attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay3Int, CHANGE);
            //     #endif
            //     instances[2] = this;
            //     break;
            // case inRelay4Pin:
            //     #if defined(ESP8266)
            //         attachInterrupt(inRelay4Pin, inRelay4Int, CHANGE);
            //     #else
            //         attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin), inRelay4Int, CHANGE);
            //     #endif
            //     instances[3] = this;
            //     break;
        }
    }
    else
        errorStatus |= ERRMODE;
}   
// Отключение прерывания
void RelayIn::offInt(){
    #if defined(ESP8266)
        detachInterrupt(pin);
    #else
        detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pin));
    #endif
}   

void RECIEVE_ATTR RelayIn::inRelay1Int(){
    if(RelayIn::instances[0] != NULL ){
        RelayIn::instances[0]->intRising();
    }
}

void RECIEVE_ATTR RelayIn::inRelay2Int(){
    if(RelayIn::instances[1] != NULL ){
        RelayIn::instances[1]->intRising();
    }
}

void RECIEVE_ATTR RelayIn::inRelay3Int(){
    if(RelayIn::instances[2] != NULL ){
        RelayIn::instances[2]->intRising();
    }
}

void RECIEVE_ATTR RelayIn::inRelay4Int(){
    if(RelayIn::instances[3] != NULL ){
        RelayIn::instances[3]->intRising();
    }
}

void RelayIn::intRising(){
    intFlag = digitalRead(pin);
}

bool RelayIn::getInt(){
    return intFlag;
}