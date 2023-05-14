#include <Arduino.h>

class Motor {
    private:
        byte pinMotor; // Ножка насоса
        byte errorMotor; // Байт ошибок и статусов насоса
        bool statusMotor; // Текущий статус насоса (Вкл/Выкл)
        uint32_t currentTime; // Текущее время
        bool bDelay; // Бит начала работы таймера    
    public:
        Motor();
        Motor(byte ePin, bool eStatus);
        // Включить насос
        void onMotor();
        // Включить насос с задержкой
        void onMotor(uint32_t _delay);
        // Выключить насос
        void offMotor();
        // Статус насоса
        bool getStatusMotor();
        // Функция получения ошибки
        byte getError();
        // Функция очистки ошибок
        void clearError();
};