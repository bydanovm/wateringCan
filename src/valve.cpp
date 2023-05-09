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
    // if(errorValve == 0x00){
        if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
            statusValve = true;
            digitalWrite(pinValve, statusValve);
            // errorValve = 0x01;
        }
        // else{
        //     errorValve = 0xF1;
        // }
    // }
}
// Функция закрытия клапана
void Valve::closeValve(){
    // if(errorValve == 0x00){
        if(statusValve == true){
            statusValve = false;
            digitalWrite(pinValve, statusValve);
            // errorValve = 0x02;
        }
        remoteControl = false;
        // else{
        //     errorValve = 0xF2;
        // }
    // }
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
// Функция установки разрешения на открытие
void Valve::setPermitionOpenValve(){
    permitionOpenValve = true;
}
// Функция снятия разрешения на открытие
void Valve::unsetPermitionOpenValve(){
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