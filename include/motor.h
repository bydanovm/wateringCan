#include <Arduino.h>

class Motor {
    private:
        byte pinMotor; // Ножка насоса
        byte errorMotor; // Байт ошибок и статусов насоса
        bool statusMotor; // Текущий статус насоса (Вкл/Выкл)
    public:
        Motor();
        Motor(byte ePin, bool eStatus);
        // Включить насос
        void onMotor();
        // Выключить насос
        void offMotor();
        // Статус насоса
        bool getStatusMotor();
        // Функция получения ошибки
        byte getError();
        // Функция очистки ошибок
        void clearError();
};