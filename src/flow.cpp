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
    pinMode(pinFlowMeter, INPUT);
    // digitalWrite(pinFlowMeter, HIGH); 
}
// Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
FlowMeter::FlowMeter(byte ePin, bool eStatus){
    pinFlowMeter = ePin;
    statusFlowMeter = eStatus;
    flowRate = 0;
    errorFlow = 0x00;
    pinMode(pinFlowMeter, INPUT);
    // digitalWrite(pinFlowMeter, HIGH); 
}
// Здесь добавляем прерывания для всех расходомеров
void FlowMeter::countFlow1(){
    if(FlowMeter::instances[0] != NULL ){
        FlowMeter::instances[0]->countFlow();
    }
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
    // Serial.println(pinFlowMeter);
    switch(pinFlowMeter){
        case flowSensor1Pin:
            attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter), countFlow1, RISING);
            // Serial.println("interrupt on");
            instances[0] = this;
            break;
        case flowSensor2Pin:
            attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter), countFlow2, RISING);
            instances[1] = this;
            break;
        case flowSensor3Pin:
            attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter), countFlow3, RISING);
            instances[2] = this;
            break;
        case flowSensor4Pin:
            attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter), countFlow4, RISING);
            instances[3] = this;
            break;
    }
    currentTime = millis();
    // loopTime = currentTime;
}   
// Выключение прерывания для расходомера
void FlowMeter::offIntFlowMeter(){
    // detachInterrupt(digitalPinToInterrupt(pinFlowMeter));
    detachPinChangeInterrupt(digitalPinToPinChangeInterrupt(pinFlowMeter));
}   
// Функция расчетов, возвращает бит изменения значения
bool FlowMeter::calcRateVolume(){
    bool result = false;
    // Serial.println("statusFlowMeter: " + (String)statusFlowMeter);
    if(statusFlowMeter == true){
        // currentTime = millis();
        if (millis() >= (currentTime + cTime)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
        {
            if(flowFreq != 0){
                currentTime = millis();
                // Serial.println("flowFreq: " + (String)flowFreq);
                // Умножаем на 10, чтобы не было целой части в расчетах (экономим память и быстродействие)
                tempFlowRate = flowFreq * uint16_t(cFlowRatePule * 10); // Имеется погрешность при преобразовании типов в расчётах
                // Serial.println("tempFlowRate: " + (String)tempFlowRate);
                prevFlowRate = tempFlowRate * 60; // Частота * cFlowRatePule * 60 = мл/мин
                // Выводим значение по изменению
                if (prevFlowRate != flowRate){
                    flowRate = prevFlowRate;
                }
                flowVolume += tempFlowRate; // Расчет объема - мл
                flowFreq = 0; // Сбрасываем счетчик тиков
                result = true;
            }else{
                result = false;
            }
        }
    }
    else{
        flowRate = 0;
        flowVolume = 0;
    }
    // Формирование события о максимальном объёме
    if(flowVolume > maxVolume * 10){
        errorFlow |= eMaxVolume;
        // Serial.println("MAX" + (String)errorFlow);
    }
    return result;
}
// Функция получения расхода
uint32_t FlowMeter::getFlowRate(){
    return flowRate / 10;
}
// Функция получения объема
uint32_t FlowMeter::getVolume(){
    return flowVolume / 10;
}
// Функция установки максимального объема
bool FlowMeter::setMaxVolume(uint32_t _maxVolume){
    maxVolume = _maxVolume;
    return true;
}
// Возврат максимального обьема
uint32_t FlowMeter::getMaxVolume(){
    return maxVolume;
}
// Включить вычисление расходомера и обнулить данные по обьему
void FlowMeter::onFlowMeter(){
    flowVolume = 0;
    statusFlowMeter = true;
}
void FlowMeter::onFullFlowMeter(){
    onIntFlowMeter();
    onFlowMeter();
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