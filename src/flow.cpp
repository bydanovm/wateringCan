// #include <Arduino.h>
#include "flow.h"
// Создаем конструкторы класса, для начальной установки
// Если создается пустой класс, то по умолчанию задается ножка 18 на вход
FlowMeter::FlowMeter(){
    pinFlowMeter = 18;
    statusFlowMeter = false;
    flowRate = 0;
    pinMode(pinFlowMeter, INPUT_PULLUP);
    digitalWrite(pinFlowMeter, HIGH); 
}
// Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
FlowMeter::FlowMeter(byte ePin, bool eStatus){
    pinFlowMeter = ePin;
    statusFlowMeter = eStatus;
    flowRate = 0;
    pinMode(pinFlowMeter, INPUT_PULLUP);
    digitalWrite(pinFlowMeter, HIGH); 
}
// Включение прерывания для расходомера
void FlowMeter::onIntFlowMeter(){
    attachInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow, HIGH);
    currentTime = millis();
    loopTime = currentTime;
}   
// Выключение прерывания для расходомера
void FlowMeter::offIntFlowMeter(){
    detachInterrupt(digitalPinToInterrupt(pinFlowMeter));
}   
// Функция расчетов
void FlowMeter::calcRateVolume(){
    if(statusFlowMeter == true){
        currentTime = millis();
        if (currentTime >= (loopTime + 1000))
        {
            loopTime = currentTime;
            flowRate = flowFreq / cFlowRatePule; // Частота / cFlowRatePule = расход в л/мин
            flowVolume += flowRate / 60; // Расчет объема
            flowFreq = 0; // Сбрасываем счетчик
        }
    }
    else{
        flowRate = 0;
        flowVolume = 0;
    }
    if(flowVolume > maxVolume){
        errorValve |= eMaxVolume;
    }
}
// Функция получения расхода
uint16_t FlowMeter::getFlowRate(){
    return flowRate;
}
// Функция получения объема
uint16_t FlowMeter::getVolume(){
    return flowVolume;
}
// Функция установки максимального объема
void FlowMeter::setMaxVolume(uint16_t _maxVolume){
    maxVolume = _maxVolume;
}
// Включить вычисление расходомера
void FlowMeter::onFlowMeter(){
    statusFlowMeter = true;
}
// Выключить вычисление расходомера
void FlowMeter::offFlowMeter(){
    statusFlowMeter = false;
}
// Функция получения ошибки
byte FlowMeter::getError(){
    return errorValve;
}
// Функция очистки ошибок
void FlowMeter::clearError(){
    errorValve = 0x00;
}