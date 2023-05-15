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
    if(statusMotor == false){
        statusMotor = true;
        digitalWrite(pinMotor, statusMotor);
        Serial.println("MOTOR ON");
    }
}
// Включить насос с задержкой
void Motor::onMotor(uint32_t _delay){
    if(statusMotor == false){
        if(bDelay == false){
            bDelay = true;
            currentTime = millis();
            // Serial.println(currentTime);
        }
        if (millis() >= (currentTime + _delay)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
        {
            statusMotor = true;
            digitalWrite(pinMotor, statusMotor);
            bDelay = false;
            Serial.println("MOTOR ON");
        }
    }
}
// Выключить насос
void Motor::offMotor(){
    if(statusMotor == true){
        statusMotor = false;
        digitalWrite(pinMotor, statusMotor);
        Serial.println("MOTOR OFF");
    }
}
// Выключить насос по задержке
void Motor::offMotor(uint32_t _delay){
    if(bDelay == false){
        bDelay = true;
        currentTime = millis();
        // Serial.println(currentTime);
    }
    if (millis() >= (currentTime + _delay)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
    {
        if(statusMotor == true){
            statusMotor = false;
            digitalWrite(pinMotor, statusMotor);
            bDelay = false;
            Serial.println("MOTOR OFF");
        }
    }
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