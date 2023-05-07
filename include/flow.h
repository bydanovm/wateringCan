#include <Arduino.h>
#define cFlowRatePule 7.5 // Уточнить константу
// События
#define eMaxVolume 0xE1 // Достигнут максимальный объем
// Создаем класс расходомера
class FlowMeter{
    // Задаем переменные и функции, которые будут доступны внутри класса (см. Инкапсуляция)
    private:
        byte pinFlowMeter; // Ножка клапана
        byte errorValve; // Байт ошибок и статусов клапана
        bool statusFlowMeter; // Текущий статус расходомера (Вкл/Выкл)
        uint16_t flowRate; // Расход
        uint16_t flowVolume; // Литр/час
        uint32_t currentTime; // Текущее время
        uint32_t loopTime; // Время цикла
        uint16_t maxVolume;

        static void countFlow();
    // Задаем переменные и функции, которые будут доступны вне класса
    public:        
        static volatile uint16_t flowFreq; // Частота
        // Создаем конструкторы класса, для начальной установки
        // Если создается пустой класс, то по умолчанию задается ножка 18 на вход
        FlowMeter();
        // Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
        FlowMeter(byte ePin, bool eStatus);
        // Включение прерывания для расходомера
        void onIntFlowMeter();
        // Выключение прерывания для расходомера
        void offIntFlowMeter();
        // Функция расчетов
        void calcRateVolume();
        // Функция получения расхода
        uint16_t getFlowRate();
        // Функция получения объема
        uint16_t getVolume();
        // Функция установки максимального объема
        void setMaxVolume(uint16_t _maxVolume);
        // Включить вычисление расходомера
        void onFlowMeter();
        // Выключить вычисление расходомера
        void offFlowMeter();
        // Функция получения ошибки
        byte getError();
        // Функция очистки ошибок
        void clearError();
};