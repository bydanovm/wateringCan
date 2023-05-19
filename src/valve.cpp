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
bool Valve::openValve(){
    bool result = false;
    if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
        statusValve = true;
        digitalWrite(pinValve, statusValve);
        if(getCountOpenValve() < countObjects)
            staticCountOpenValveIncr();
        result = true;
    }
    return result;
}
// Функция открытия клапана с задержкой
bool Valve::openValve(uint32_t _delay){
    bool result = false;
    if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
        if(bDelay == false){
            bDelay = true;
            currentTime = millis();
            // Serial.println(currentTime);
        }
        if (millis() >= (currentTime + _delay)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
        {
            statusValve = true;
            digitalWrite(pinValve, statusValve);
            if(getCountOpenValve() < countObjects)
                staticCountOpenValveIncr();
            // currentTime = 0;
            bDelay = false;
            // Serial.println("VALVE OPEN");
            result = true;
        }
    }
    return result;
}
// Функция закрытия клапана
bool Valve::closeValve(){
    bool result = false;
    if(statusValve == true){
        statusValve = false;
        digitalWrite(pinValve, statusValve);
        if(getCountOpenValve() > 0)
            staticCountOpenValveDecr();
        // Serial.println("VALVE CLOSE");
        result = true;
    }
    remoteControl = false;
    return result;
}
// Экстренное открытие клапана
bool Valve::extOpenValve(){
    bool result = false;
    remoteControl = true;
    if(openValve())
        result = true;
    return result;
}
// Функция получения состояния клапана
bool Valve::getStatusValve(){
    return statusValve;
}
// Статическая функция получения количества клапанов для работы
bool Valve::getPermitionOpenValve(){
    return permitionOpenValve;
}
// Статическая функция инкрементирования количества клапанов для работы
void Valve::staticCountPermValveIncr(){
    staticCountPermValve++;
}
// Статическая функция декрементирования количества клапанов для работы
void Valve::staticCountPermValveDecr(){
    staticCountPermValve--;
}
// Статическая функция инкрементирования количества открытых клапанов
void Valve::staticCountOpenValveIncr(){
    staticCountOpenValve++;
}
// Статическая функция декрементирования количества открытых клапанов
void Valve::staticCountOpenValveDecr(){
    staticCountOpenValve--;
}
// Функция установки разрешения на открытие
void Valve::setPermitionOpenValve(){
    if(!getPermitionOpenValve()){
        if(getCountPermValve() < countObjects)
            staticCountPermValveIncr();
        permitionOpenValve = true;
    }
}
// Функция снятия разрешения на открытие
void Valve::unsetPermitionOpenValve(){
    if(getPermitionOpenValve()){
        if(getCountPermValve()  > 0)
            staticCountPermValveDecr();
        permitionOpenValve = false;
    }
}
// Функция получения ошибки
byte Valve::getError(){
    return errorValve;
}
// Функция очистки ошибок
void Valve::clearError(){
    errorValve = 0x00;
}
// Функция получения количества клапанов для работы
uint8_t Valve::getCountPermValve(){
    return staticCountPermValve;
}
// Функция получения количества открытых клапанов
uint8_t Valve::getCountOpenValve(){
    return staticCountOpenValve;
}
// Инициализация статических переменных
uint8_t Valve::staticCountOpenValve = 0;
uint8_t Valve::staticCountPermValve = 0;