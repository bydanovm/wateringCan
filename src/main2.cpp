#include <Arduino.h>
// Подключаем библиотеки
#include "Nextion.h"
#include "PinChangeInterrupt.h"

//думаю также нужно подключить библиотеки Arduino_Nextion и FlowMeter

// Определяем пины для управления клапанами и насосом
#define valve1Pin 2
#define valve2Pin 3
#define valve3Pin 4
#define valve4Pin 5
#define pumpPin 6
#define flowSensor1Pin 7
#define flowSensor2Pin 8
#define flowSensor3Pin 9
#define flowSensor4Pin 10

// Определяем пины для кнопок "Пуск" и "Стоп"
#define buttonPinStart 11
#define buttonPinStop 12

#define cFlowRatePule 1.2 // Мл/П

// Определяем переменные обрабатывания кнопок "Пуск" и "Стоп"
volatile bool buttonStateStart = false;
volatile bool buttonStateStop = false;
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
volatile int pulseCount3 = 0;
volatile int pulseCount4 = 0;

// Определяем начальное время
bool bDelay   = false;
unsigned int startOpenTime = 0;
unsigned int stopCloseTime = 0;
unsigned int startCloseTime = 0;
unsigned int startPumpTime = 0;
unsigned int calcTime = 0; // Время начала расчета расхода
unsigned int flowRate1 = 0; // переменная для хранения текущей скорости потока
unsigned int totalVolume1 = 0; // переменная для хранения общего объема жидкости
unsigned int flowRate2 = 0; // переменная для хранения текущей скорости потока
unsigned int totalVolume2 = 0; // переменная для хранения общего объема жидкости
unsigned int flowRate3 = 0; // переменная для хранения текущей скорости потока
unsigned int totalVolume3 = 0; // переменная для хранения общего объема жидкости
unsigned int flowRate4 = 0; // переменная для хранения текущей скорости потока
unsigned int totalVolume4 = 0; // переменная для хранения общего объема жидкости

// Определяем задержки в миллисекундах
#define valveOpenDelay 500 // задержка при открытии клапана
#define valveCloseDelay 2000 // задержка при закрытии клапана
#define pumpStartDelay 200 // задержка при запуске насоса
#define pumpStopDelay 1000 // задержка при остановке насоса

// Определяем переменные для считывания данных с дисплея
int num1ValMax = 0;
int num2ValMax = 0;
int num3ValMax = 0;
int num4ValMax = 0;
bool btn1State = false;
bool btn2State = false;
bool btn3State = false;
bool btn4State = false;

//Определяем флаг состояния работы программы
bool isRunning = false; // Флаг запуска системы

// Определяем объект для работы с дисплеем
Nextion myNextion(Serial2, 9600);

// Объявляем функции
void setValve(int valve, int state);
void startPump();
void stopPump();
void flushValves();
void updateValues();
void controlSystem();
void pulseCounter1();
void pulseCounter2();
void pulseCounter3();
void pulseCounter4();

bool isAllValvesOpened();
bool isAllValvesClosed();

void getFlowSensorValue(unsigned int timeDelay = 1000);

void setup() {
    // Настроим пины на выход
    pinMode(valve1Pin, OUTPUT);
    pinMode(valve2Pin, OUTPUT);
    pinMode(valve3Pin, OUTPUT);
    pinMode(valve4Pin, OUTPUT);
    pinMode(pumpPin, OUTPUT);

    // Настраиваем пины на входы с включенными прерываниями
    pinMode(buttonPinStart, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(buttonPinStart), startButtonPressed, RISING);
    pinMode(buttonPinStop, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(buttonPinStop), stopButtonPressed, RISING);
    pinMode(flowSensor1Pin, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(flowSensor1Pin), pulseCounter1, RISING);
    pinMode(flowSensor2Pin, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(flowSensor2Pin), pulseCounter2, RISING);
    pinMode(flowSensor3Pin, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(flowSensor3Pin), pulseCounter3, RISING);
    pinMode(flowSensor4Pin, INPUT);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(flowSensor4Pin), pulseCounter4, RISING);

    // Инициализируем Serial для отладки
    Serial.begin(9600); 

    // Инициализируем дисплей
    myNextion.beginCom();
    myNextion.init();

    // Передаем значения с дисплея в переменные
    updateValues();
}

void loop() {
    // Входящие сообщения с экрана
    String message = myNextion.listen();
    if(message != ""){
        if(message == "UPDVOL" && !isRunning){ // Проверка "Система не в работе"
            // Обновляем значения с дисплея
            updateValues();
        }
        if(message == "UPDVOL" && isRunning){ // Проверка "Система в работе"
            // Обновляем значения с дисплея
            updateValues();
            myNextion.setComponentValue("num1", num1ValMax);
        }
    }

    // Управляем клапанами, расходомерами и насосом
    controlSystem();
    // Расчет расходомеров
    getFlowSensorValue();
}

// Управление клапанами
void setValve(int valve, int state) {
  switch (valve) {
    case valve1Pin:
        digitalWrite(valve1Pin, state);
        break;
    case valve2Pin:
        digitalWrite(valve2Pin, state);
        break;
    case valve3Pin:
        digitalWrite(valve3Pin, state);
        break;
    case valve4Pin:
        digitalWrite(valve4Pin, state);
        break;
    other:
        break;
  }
}

void startPump() {
  digitalWrite(pumpPin, HIGH);
}

void stopPump() {
  digitalWrite(pumpPin, LOW);
}

void flushValves() {
  // Открываем все клапаны
  setValve(valve1Pin, HIGH);
  setValve(valve2Pin, HIGH);
  setValve(valve3Pin, HIGH);
  setValve(valve4Pin, HIGH);
}

void updateValues() {
    // Считываем значения элементов
    num1ValMax = myNextion.getComponentValue("num1");
    num2ValMax = myNextion.getComponentValue("num2");
    num3ValMax = myNextion.getComponentValue("num3");
    num4ValMax = myNextion.getComponentValue("num4");
}

void controlSystem() {
    // Запуск системы
    if(!isRunning && buttonStateStart){
        if(bDelay == false){
            bDelay = true;
            startOpenTime = millis();
            // Проверяем состояние кнопок 1 РАЗ!!!
            // 1 - разрешить; 0 - запретить
            btn1State = myNextion.getComponentValue("btn1");
            btn2State = myNextion.getComponentValue("btn2");
            btn3State = myNextion.getComponentValue("btn3");
            btn4State = myNextion.getComponentValue("btn4");
        }

        // Управляем клапанами
        if (!btn1State) {
            setValve(valve1Pin, LOW);
        } else {
            if(!digitalRead(valve1Pin)){
                if (num1ValMax > 0 && (millis() - startOpenTime > valveOpenDelay)) {
                    setValve(valve1Pin, HIGH);
                }
            }
        }
        if (!btn2State) {
            setValve(valve2Pin, LOW);
        } else {
            if(!digitalRead(valve2Pin)){
                if (num2ValMax > 0 && (millis() - startOpenTime > valveOpenDelay)) {
                    setValve(valve2Pin, HIGH);
                }
            }
        }
        if (!btn3State) {
            setValve(valve3Pin, LOW);
        } else {
            if(!digitalRead(valve3Pin)){
                if (num3ValMax > 0 && (millis() - startOpenTime > valveOpenDelay)) {
                    setValve(valve3Pin, HIGH);
                }
            }
        }
        if (!btn4State) {
            setValve(valve4Pin, LOW);
        } else {
            if(!digitalRead(valve4Pin)){
                if (num4ValMax > 0 && (millis() - startOpenTime > valveOpenDelay)) {
                    setValve(valve4Pin, HIGH);
                }
            }
        }
        if(isAllValvesOpened()){ // проработать вариант открытия при не всех клапанах
            if (millis() - startOpenTime > pumpStartDelay) { // ждем, пока пройдет задержка pumpStartDelay
                startPump();
                bDelay = false;
                isRunning = true; // Флаг запуска системы
                buttonStateStart = false;
                myNextion.sendCommand("page 1");
            }
        }
    }
    // ОСТАНОВ
    if(isRunning && buttonStateStop){
        if(bDelay == false){
            bDelay = true;
            stopCloseTime = millis();
        }
        if (millis() - stopCloseTime > valveCloseDelay) {
            setValve(valve1Pin, LOW);
        }
        if (millis() - stopCloseTime > valveCloseDelay) {
            setValve(valve2Pin, LOW);
        }
        if (millis() - stopCloseTime > valveCloseDelay) {
            setValve(valve3Pin, LOW);
        }
        if (millis() - stopCloseTime > valveCloseDelay) {
            setValve(valve4Pin, LOW);
        }
        if (millis() - stopCloseTime > pumpStopDelay) { // ждем, пока пройдет задержка pumpStopDelay
            stopPump();
            isRunning = false; // Флаг запуска системы
            buttonStateStop = false;
            myNextion.sendCommand("page 0");
        }
    }  // if(isRunning && buttonStateStop)
} 
 // Расчет раходомеров
void getFlowSensorValue(unsigned int timeDelay = 1000){
    if(bDelay == false){
        bDelay = true;
        calcTime = millis();
    }
    if (millis() - calcTime > timeDelay) {
        // Умножаем на 10, чтобы не было дробной части в расчетах (экономим память и быстродействие)
        flowRate1 = pulseCount1 * uint16_t(cFlowRatePule * 10); // Расход на единицу времени timeDelay
        totalVolume1 += flowRate1 / 10; 
        flowRate2 = pulseCount2 * uint16_t(cFlowRatePule * 10); // Расход на единицу времени timeDelay
        totalVolume2 += flowRate2 / 10; 
        flowRate3 = pulseCount3 * uint16_t(cFlowRatePule * 10); // Расход на единицу времени timeDelay
        totalVolume3 += flowRate3 / 10; 
        flowRate4 = pulseCount4 * uint16_t(cFlowRatePule * 10); // Расход на единицу времени timeDelay
        totalVolume4 += flowRate4 / 10; 

        pulseCount1 = 0;
        pulseCount2 = 0;
        pulseCount3 = 0;
        pulseCount4 = 0;
    }
}

bool isAllValvesClosed() {
  if (digitalRead(valve1Pin) == LOW &&
      digitalRead(valve2Pin) == LOW &&
      digitalRead(valve3Pin) == LOW &&
      digitalRead(valve4Pin) == LOW) {
    return true;
  } else {
    return false;
  }
}

bool isAllValvesOpened() {
  if (digitalRead(valve1Pin) == HIGH &&
      digitalRead(valve2Pin) == HIGH &&
      digitalRead(valve3Pin) == HIGH &&
      digitalRead(valve4Pin) == HIGH) {
    return true;
  } else {
    return false;
  }
}
// ПРЕРЫВАНИЯ
void startButtonPressed() {
    buttonStateStart = true;
}
void stopButtonPressed() {
    buttonStateStop = true;
}
void pulseCounter1(){
    pulseCount1++;
}
void pulseCounter2(){
    pulseCount2++;
}
void pulseCounter3(){
    pulseCount3++;
}
void pulseCounter4(){
    pulseCount4++;
}