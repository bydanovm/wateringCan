// #include <Arduino.h>
#include "flow.h"
#include "PinChangeInterrupt.h"
// Создаем конструкторы класса, для начальной установки
// Если создается пустой класс, то по умолчанию задается ножка 18 на вход
FlowMeter::FlowMeter(){
    pinFlowMeter = 18;
    statusFlowMeter = false;
    flowRate = 0;
    errorFlow= 0x00;
    pinMode(pinFlowMeter, INPUT_PULLUP);
    digitalWrite(pinFlowMeter, HIGH); 
}
// Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
FlowMeter::FlowMeter(byte ePin, bool eStatus){
    pinFlowMeter = ePin;
    statusFlowMeter = eStatus;
    flowRate = 0;
    errorFlow = 0x00;
    pinMode(pinFlowMeter, INPUT_PULLUP);
    digitalWrite(pinFlowMeter, HIGH); 
}
// Здесь добавляем прерывания для всех расходомеров
void FlowMeter::countFlow1(){
    if(FlowMeter::instances[0] != NULL )
        FlowMeter::instances[0]->countFlow();
}
void FlowMeter::countFlow2(){
    if(FlowMeter::instances[1] != NULL )
        FlowMeter::instances[1]->countFlow();
}
void FlowMeter::countFlow3(){
    if(FlowMeter::instances[2] != NULL )
        FlowMeter::instances[2]->countFlow();
}
void FlowMeter::countFlow4(){
    if(FlowMeter::instances[3] != NULL )
        FlowMeter::instances[3]->countFlow();
}

void FlowMeter::countFlow(){
    flowFreq++;
}
// Включение прерывания для расходомера
void FlowMeter::onIntFlowMeter(){
    // attachInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow, HIGH);
    switch(pinFlowMeter){
        case flowSensor1:
            attachPinChangeInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow1, RISING);
            instances[0] = this;
            break;
        case flowSensor2:
            attachPinChangeInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow2, RISING);
            instances[1] = this;
            break;
        case flowSensor3:
            attachPinChangeInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow3, RISING);
            instances[2] = this;
            break;
        case flowSensor4:
            attachPinChangeInterrupt(digitalPinToInterrupt(pinFlowMeter), countFlow4, RISING);
            instances[3] = this;
            break;
    }
    // attachPCINT(pinFlowMeter);
    currentTime = millis();
    loopTime = currentTime;
}   
// Выключение прерывания для расходомера
void FlowMeter::offIntFlowMeter(){
    // detachInterrupt(digitalPinToInterrupt(pinFlowMeter));
    detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter));
}   
// ISR(PCINT0_vect) {  // пины 8-13
//     if(pinRead(FlowMeter;
// }
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
        errorFlow |= eMaxVolume;
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
    return errorFlow;
}
// Функция очистки ошибок
void FlowMeter::clearError(){
    errorFlow = 0x00;
}