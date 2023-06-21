#ifndef HEADER_FLOW
#define HEADER_FLOW

#include <Arduino.h>
#include "pins.h"

#define cFlowRatePule 1.2 // Уточнить константу
#define cTime 1000
// События
#define eMaxVolume 0xE1 // Достигнут максимальный объем
// Создаем класс расходомера
class FlowMeter{
    // Задаем переменные и функции, которые будут доступны внутри класса (см. Инкапсуляция)
    private:
        byte pinFlowMeter; // Ножка клапана
        byte errorFlow; // Байт ошибок и статусов расходомера
        bool statusFlowMeter; // Текущий статус расходомера (Вкл/Выкл)
        uint32_t tempFlowRate;
        uint32_t flowRate; // Расход
        uint32_t flowVolume; // Объем
        uint32_t prevFlowRate; // Предыдущий расход
        uint32_t prevFlowVolume; // Предыдущий объем
        uint32_t currentTime; // Текущее время
        // uint32_t loopTime; // Время цикла
        uint32_t maxVolume;

        volatile uint16_t flowFreq; // Частота
        // Топорная привязка прерываний к классу, найти более простой способ
        // Потому что для каждого клапана придется добавлять статичный метод
        // и это изначально занимает много памяти
        static FlowMeter * instances [4];
        static void countFlow1();
        static void countFlow2();
        static void countFlow3();
        static void countFlow4();
        void countFlow();
    // Задаем переменные и функции, которые будут доступны вне класса
    public:        
        // Создаем конструкторы класса, для начальной установки
        // Если создается пустой класс, то по умолчанию задается ножка 18 на вход
        FlowMeter();
        // Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
        FlowMeter(byte ePin, bool eStatus);
        // Включение прерывания для расходомера
        void onIntFlowMeter();
        // Выключение прерывания для расходомера
        void offIntFlowMeter();
        // Функция расчетов, возвращает бит изменения значения
        bool calcRateVolume();
        // Функция получения расхода
        uint32_t getFlowRate();
        // Функция получения объема
        uint32_t getVolume();
        // Функция установки максимального объема
        bool setMaxVolume(uint32_t _maxVolume);
        // Возврать максимального обьема
        uint32_t getMaxVolume();
        // Включить вычисление расходомера
        void onFlowMeter();
        // Выключить вычисление расходомера
        void offFlowMeter();
        // Включение расходомера и прерывания
        void onFullFlowMeter();
        // Функция получения ошибки
        byte getError();
        // Функция очистки ошибок
        void clearError();
};

#endif