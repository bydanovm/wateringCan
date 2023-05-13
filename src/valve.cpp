#include <valve.h>
// Создаем конструкторы класса, для начальной установки
Valve::Valve(){
    pinValve = 10;
    statusValve = false;
    errorValve = 0x00;
    permitionOpenValve = false;
    remoteControl = false;
    pinMode(pinValve, OUTPUT);
}
// Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
Valve::Valve(byte ePin, bool eStatus){
    pinValve = ePin;
    statusValve = eStatus;
    errorValve = 0x00;
    permitionOpenValve = false;
    remoteControl = false;            
    pinMode(pinValve, OUTPUT);
}
// Функция открытия клапана
void Valve::openValve(){
    if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
        statusValve = true;
        digitalWrite(pinValve, statusValve);
        // if(countOpenValve < countObjects)
        //     countOpenValve++;
    }
}
// Функция открытия клапана с задержкой
void Valve::openValve(uint8_t _delay){
    if(bDelay == false){
        bDelay = true;
        currentTime = millis();
    }
    if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
        if (millis() >= (currentTime + _delay)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
        {
            statusValve = true;
            digitalWrite(pinValve, statusValve);
            if(getCountOpenValve() < countObjects)
                staticCountOpenValveIncr();
            bDelay = false;
        }
    }
}
// Функция закрытия клапана
void Valve::closeValve(){
    if(statusValve == true){
        statusValve = false;
        digitalWrite(pinValve, statusValve);
        if(getCountOpenValve() > 0)
            staticCountOpenValveDecr();
    }
    remoteControl = false;
}
// Экстренное открытие клапана
void Valve::extOpenValve(){
    remoteControl = true;
    openValve();
}
// Функция получения состояния клапана
bool Valve::getStatusValve(){
    return statusValve;
}
bool Valve::getPermitionOpenValve(){
    return permitionOpenValve;
}
void Valve::staticCountPermValveIncr(){
    staticCountPermValve++;
}
void Valve::staticCountPermValveDecr(){
    staticCountPermValve--;
}
void Valve::staticCountOpenValveIncr(){
    staticCountOpenValve++;
}
void Valve::staticCountOpenValveDecr(){
    staticCountOpenValve--;
}
// Функция установки разрешения на открытие
void Valve::setPermitionOpenValve(){
    if(getCountPermValve() < countObjects)
        staticCountPermValveIncr();
    permitionOpenValve = true;
}
// Функция снятия разрешения на открытие
void Valve::unsetPermitionOpenValve(){
    if(getCountPermValve()  > 0)
        staticCountPermValveDecr();
    permitionOpenValve = false;
}
// Функция получения ошибки
byte Valve::getError(){
    return errorValve;
}
// Функция очистки ошибок
void Valve::clearError(){
    errorValve = 0x00;
}
uint8_t Valve::getCountPermValve(){
    return staticCountPermValve;
}
uint8_t Valve::getCountOpenValve(){
    return staticCountOpenValve;
}
uint8_t Valve::staticCountOpenValve = 0;
uint8_t Valve::staticCountPermValve = 0;