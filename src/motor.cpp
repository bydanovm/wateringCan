#include "motor.h"
Motor::Motor(){
    pinMotor = 5;
    statusMotor = false;
    errorMotor = 0x00;
    pinMode(pinMotor, OUTPUT);
}
Motor::Motor(byte ePin, bool eStatus){
    pinMotor = ePin;
    statusMotor = eStatus;
    errorMotor = 0x00;
    pinMode(pinMotor, OUTPUT);
}
// Включить насос
void Motor::onMotor(){
    // if(errorMotor == 0x00){
        if(statusMotor == false){
            statusMotor = true;
            digitalWrite(pinMotor, statusMotor);
            // errorMotor = 0x01;
        }
        // else{
        //     errorMotor = 0xF1;
        // }
    // }
}
// Выключить насос
void Motor::offMotor(){
    // if(errorMotor == 0x00){
        if(statusMotor == true){
            statusMotor = false;
            digitalWrite(pinMotor, statusMotor);
            // errorMotor = 0x00;
        }
        // else{
        //     errorMotor = 0xE1;
        // }
    // }
}
bool Motor::getStatusMotor(){
    return statusMotor;
}
// Функция получения ошибки
byte Motor::getError(){
    return errorMotor;
}
// Функция очистки ошибок
void Motor::clearError(){
    errorMotor = 0x00;
}