#include <Arduino.h>
// Создаем класс клапана
class Valve {
    // Задаем переменные и функции, которые будут доступны внутри класса (см. Инкапсуляция)
    private:
        byte pinValve; // Ножка клапана
        bool statusValve; // Текущий статус клапана (О/З)
        bool permitionOpenValve; // Разрешение на открытие клапана
        bool remoteControl; // Внешнее управление
        byte errorValve; // Байт ошибок и статусов клапана
        
    // Задаем переменные и функции, которые будут доступны вне класса
    public:
        // Создаем конструкторы класса, для начальной установки
        // Если создается пустой класс, то по умолчанию задается ножка 10 на выход
        Valve(){
            pinValve = 10;
            statusValve = false;
            errorValve = 0x00;
            permitionOpenValve = false;
            remoteControl = false;
            pinMode(pinValve, OUTPUT);
        }
        // Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
        Valve(byte ePin, bool eStatus){
            pinValve = ePin;
            statusValve = eStatus;
            errorValve = 0x00;
            permitionOpenValve = false;
            remoteControl = false;            
            pinMode(pinValve, OUTPUT);
        }
        // Функция открытия клапана
        void openValve(){
            if(errorValve == 0x00){
                if(((statusValve == false) && (permitionOpenValve == true)) || (remoteControl == true)){
                    statusValve = true;
                    digitalWrite(pinValve, statusValve);
                    errorValve = 0x01;
                }
                else{
                    errorValve = 0xF1;
                }
            }
        }
        // Функция закрытия клапана
        void closeValve(){
            if(errorValve == 0x00){
                if(statusValve == true){
                    statusValve = false;
                    digitalWrite(pinValve, statusValve);
                    errorValve = 0x02;
                }
                else{
                    errorValve = 0xF2;
                }
            }
        }
        // Функция получения состояния клапана
        bool getStatusValve(){
            return statusValve;
        }
        // Функция установки разрешения на открытие
        void setPermitionOpenValve(){
            permitionOpenValve = true;
        }
        // Функция снятия разрешения на открытие
        void unsetPermitionOpenValve(){
            permitionOpenValve = false;
        }
        // Функция получения ошибки
        byte getError(){
            return errorValve;
        }
        // Функция очистки ошибок
        void clearError(){
            errorValve = 0x00;
        }
};