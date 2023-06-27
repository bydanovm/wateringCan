#ifndef HEADER_FLOW
#define HEADER_FLOW

#include <Arduino.h>
#include "pins.h"

#define cFlowRatePule 1.4 // Уточнить константу
#define cTime 50
#define volumeIsLiter 1200
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
        float flowVolume; // Объем
        uint32_t flowVolumeInt; // Объем
        float prevFlowRate; // Предыдущий расход
        uint32_t prevFlowVolume; // Предыдущий объем
        uint32_t currentTime; // Текущее время
        uint32_t loopTime; // Время опроса датчика
        uint32_t maxVolume;
        float koeff; // Коэффициент преобразования

        volatile uint16_t flowFreq; // Частота
        // Топорная привязка прерываний к классу, найти более простой способ
        // Потому что для каждого клапана придется добавлять статичный метод
        // и это изначально занимает много памяти
        static FlowMeter * instances [4];
        static void countFlow1();
        static void countFlow2();
        static void countFlow3();
        static void countFlow4();
    // Задаем переменные и функции, которые будут доступны вне класса
    public:        
        // Создаем конструкторы класса, для начальной установки
        // Если создается пустой класс, то по умолчанию задается ножка 18 на вход
        FlowMeter();
        // Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
        FlowMeter(byte ePin, bool eStatusm, uint32_t eTimeLoop = 1000U);
        FlowMeter(byte ePin, bool eStatus, float eKoeff, uint32_t eTimeLoop = 1000U);
        // Включение прерывания для расходомера
        void onIntFlowMeter();
        // Включение прерывания для расходомера
        void beginFlowMeter(void (*userFunc)(void));
        // Выключение прерывания для расходомера
        void offIntFlowMeter();
        // Функция расчетов, возвращает бит изменения значения
        bool calcRateVolume();
        bool calcRateVolumeNew(int32_t calibration = 0);
        // Функция получения расхода
        uint32_t getFlowRate();
        // Функция получения объема
        uint32_t getVolume(uint32_t mult = 1U);
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
        void countFlow();
        uint16_t totalFlowFreq; // Частота
        union convFI {
            float f;
            uint32_t i;
        } convFI;
};

#endif