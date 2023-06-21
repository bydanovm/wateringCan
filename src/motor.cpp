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
bool Motor::onMotor(){
    bool result = false;
    if(statusMotor == false){
        statusMotor = true;
        digitalWrite(pinMotor, statusMotor);
        // Serial.println("MOTOR ON");
        result = true;
    }
    return result;
}
// Включить насос с задержкой
bool Motor::onMotor(uint32_t _delay){
    bool result = false;
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
            // Serial.println("MOTOR ON");
            result = true;
        }
    }
    return result;
}
// Выключить насос
bool Motor::offMotor(){
    bool result = false;
    if(statusMotor == true){
        statusMotor = false;
        digitalWrite(pinMotor, statusMotor);
        // Serial.println("MOTOR OFF");
        result = true;
    }
    return result;
}
// Выключить насос по задержке
bool Motor::offMotor(uint32_t _delay){
    bool result = false;
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
            // Serial.println("MOTOR OFF");
            result = true;
        }
    }
    return result;
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