#include <Arduino.h>

#include "command.h"
#include "valve.h"
#include "flow.h"
#include "motor.h"

bool bStart = false;
bool bClearing = false;
bool bStop = false;
bool flStartOperation = false;
bool flEmergencyStop = false;
bool bStartClear = false;
bool bStartValve = false;
uint32_t currentTime, loopTime = 0;
String bufStr;
// Инициализация объекта расходомера с вызовом конструктора класса FlowMeter(byte ePin, bool eStatus)
// Очишаем экземпляры классов для 4 расходомеров
FlowMeter * FlowMeter::instances[4] = {NULL, NULL};
// Ножки расходомеров настраивать в flow.h
FlowMeter flowMeter1(flowSensor1, false);
FlowMeter flowMeter2(flowSensor2, false);
FlowMeter flowMeter3(flowSensor3, false);
FlowMeter flowMeter4(flowSensor4, false);
// Инициализация объекта клапана с вызовом конструктора класса Valve(byte ePin, bool eStatus)
Valve valve1(6, false);
Valve valve2(7, false);
Valve valve3(8, false);
Valve valve4(9, false);

Motor motor1(13, false);


// Прототип функции НАЧАЛО РАБОТЫ
void startOperation();

void setup() {
  Serial.begin(9600);
  currentTime = millis();
  loopTime = currentTime;
  // pinMode(10, OUTPUT);
  // Включение расходомера и прерывания на старте 
  delay(100); // Задержка при пуске для выравнивания напряжения на ножках МК
  flowMeter1.onFullFlowMeter();
  flowMeter2.onFullFlowMeter();
  flowMeter3.onFullFlowMeter();
  flowMeter4.onFullFlowMeter();

  valve1.setPermitionOpenValve();
  valve2.setPermitionOpenValve();
  valve3.setPermitionOpenValve();
  valve4.setPermitionOpenValve();
}
void loop() {
  // currentTime = millis();
  if(Serial.available()>0){
    bufStr = Serial.readString();
    Serial.println(bufStr);
    // Данные с дисплея по UART
    // Считываем команды от дисплея и парсим их
    for(int i=0; i<bufStr.length(); i++){
      if(memcmp(&bufStr[i],V1PERM,sizeof(V1PERM))==0){
        valve1.setPermitionOpenValve();
        // Serial.println(valve1.getCountPermValve());
        // Serial.println(valve1.countObjects);
      }
      if(memcmp(&bufStr[i],V1UNPERM,sizeof(V1UNPERM))==0)
        valve1.unsetPermitionOpenValve();
      if(memcmp(&bufStr[i],V2PERM,sizeof(V2PERM))==0)
        valve2.setPermitionOpenValve();
      if(memcmp(&bufStr[i],V2UNPERM,sizeof(V2UNPERM))==0){
        valve2.unsetPermitionOpenValve();
        // Serial.println(valve1.getCountPermValve());
        // Serial.println(valve1.countObjects);     
      }   
      if(memcmp(&bufStr[i],V3PERM,sizeof(V3PERM))==0)
        valve3.setPermitionOpenValve();
      if(memcmp(&bufStr[i],V3UNPERM,sizeof(V3UNPERM))==0)
        valve3.unsetPermitionOpenValve();
      if(memcmp(&bufStr[i],V4PERM,sizeof(V4PERM))==0)
        valve4.setPermitionOpenValve();
      if(memcmp(&bufStr[i],V4UNPERM,sizeof(V4UNPERM))==0)
        valve4.unsetPermitionOpenValve();
      if(memcmp(&bufStr[i],START,sizeof(START))==0){
        bStart = true;
        bStop  = false;
      }
      if(memcmp(&bufStr[i],STOP,sizeof(STOP))==0){
        bStart = false;
        flStartOperation = false;
        bStop  = true;
      }
      if(memcmp(&bufStr[i],CLEAR,sizeof(CLEAR))==0){
        bClearing = true;
      }
    }
  }

  // НАЧАЛО РАБОТЫ
  startOperation();

  // ПРОМЫВКА
  if(bClearing == true){
    bClearing = false;
    // flowMeter1.onIntFlowMeter();
    // flowMeter1.onFlowMeter();
    // flowMeter2.onIntFlowMeter();
    // flowMeter2.onFlowMeter();
    // flowMeter3.onIntFlowMeter();
    // flowMeter3.onFlowMeter();
    // flowMeter4.onIntFlowMeter();
    // flowMeter4.onFlowMeter();
    valve1.extOpenValve();
    valve2.extOpenValve();
    valve3.extOpenValve();
    valve4.extOpenValve();
    motor1.onMotor();
    Serial.println("CLEARING ON");
  }

  if(bStop == true){
    bStop = false;
    flStartOperation = false;
    // Закрываем клапаны
    valve1.closeValve();
    valve2.closeValve();
    valve3.closeValve();
    valve4.closeValve();

    if(!valve1.getStatusValve())
      Serial.println("VALVE1 CLOSE");
    if(!valve2.getStatusValve())
      Serial.println("VALVE2 CLOSE");
    if(!valve3.getStatusValve())
      Serial.println("VALVE3 CLOSE");
    if(!valve4.getStatusValve())
      Serial.println("VALVE4 CLOSE");

    // // Выключаем расходомеры
    // flowMeter1.offFlowMeter();
    // flowMeter2.offFlowMeter();
    // flowMeter3.offFlowMeter();
    // flowMeter4.offFlowMeter();
    // // Выключение прерывания на расходомерах
    // flowMeter1.offIntFlowMeter();
    // flowMeter2.offIntFlowMeter();
    // flowMeter3.offIntFlowMeter();
    // flowMeter4.offIntFlowMeter();

    motor1.offMotor();
    if(!motor1.getStatusMotor())
      Serial.println("MOTOR OFF");
  }

  // Выполняем расчет расхода на расходомерах
  if(flowMeter1.calcRateVolume()){
    Serial.println("FLOW1: " + (String)flowMeter1.getFlowRate());
    Serial.println("VOLUME1: " + (String)flowMeter1.getVolume());
  }
  if(flowMeter2.calcRateVolume()){
    Serial.println("FLOW2: " + (String)flowMeter2.getFlowRate());
    Serial.println("VOLUME2: " + (String)flowMeter2.getVolume());
  }
  if(flowMeter3.calcRateVolume()){
    Serial.println("FLOW3: " + (String)flowMeter3.getFlowRate());
    Serial.println("VOLUME3: " + (String)flowMeter3.getVolume());
  }
  if(flowMeter4.calcRateVolume()){
    Serial.println("FLOW4: " + (String)flowMeter4.getFlowRate());
    Serial.println("VOLUME4: " + (String)flowMeter4.getVolume());
  }

  // Произошло событие "Сделан максимальный объем на расходомере 1"
  if((flowMeter1.getError() & eMaxVolume) == true && flStartOperation){
    valve1.closeValve();
    if(!valve1.getStatusValve())
      Serial.println("VALVE1 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 2"
  if((flowMeter2.getError() & eMaxVolume) == true && flStartOperation){
    valve2.closeValve();
    if(!valve2.getStatusValve())
      Serial.println("VALVE2 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 3"
  if((flowMeter3.getError() & eMaxVolume) == true && flStartOperation){
    valve3.closeValve();
    if(!valve3.getStatusValve())
      Serial.println("VALVE3 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 4"
  if((flowMeter4.getError() & eMaxVolume) == true && flStartOperation){
    valve4.closeValve();
    if(!valve4.getStatusValve())
      Serial.println("VALVE4 CLOSE MAX");
  }
  // Выключение насоса по последнему закрывшемуся клапану
  if(bStart == true && motor1.getStatusMotor() && 
      !(valve1.getStatusValve() || valve2.getStatusValve() ||
      valve3.getStatusValve() || valve4.getStatusValve())){
    motor1.offMotor();
    if(!motor1.getStatusMotor())
      Serial.println("MOTOR OFF MAX");
  }
  if (flEmergencyStop == true){
    bStart = false;
    flStartOperation = false;
    bStop  = true;
  }
}

// НАЧАЛО РАБОТЫ
void startOperation(){
  if(bStart == true && flStartOperation == false){
    // Получено значение начать операцию и операция еще не начата
    if(bStartClear == false){ 
      // Сброс ошибок на расходомерах
      flowMeter1.clearError();
      flowMeter2.clearError();
      flowMeter3.clearError();
      flowMeter4.clearError();
      // Сброс значений на расходомерах
      flowMeter1.onFlowMeter();
      flowMeter2.onFlowMeter();
      flowMeter3.onFlowMeter();
      flowMeter4.onFlowMeter();
      
      if(valve1.getPermitionOpenValve())
        Serial.println("VALVE1 READY");
      if(valve2.getPermitionOpenValve())
        Serial.println("VALVE2 READY");
      if(valve3.getPermitionOpenValve())
        Serial.println("VALVE3 READY");
      if(valve4.getPermitionOpenValve())
        Serial.println("VALVE4 READY");
      bStartClear = true;
    }

    // Открытие задвижек
    if(bStartValve == false){
      // Если количество разрешенных клапанов больше одного
      if(valve1.getCountPermValve() >= 1){
        // Открываем клапаны которым выдано разрешение
        valve1.openValve((uint32_t)250);
        valve2.openValve((uint32_t)500);
        valve3.openValve((uint32_t)750);
        valve4.openValve((uint32_t)1000);
        // Если количество открытых клапанов равно количеству разрешенных,
        // то переходим к следующему шагу запуска насоса
        if(valve1.getCountOpenValve() == valve1.getCountPermValve()){
          bStartValve = true;
        }
      }
      // Иначе - сбрасываем всё
      else {
        bStart = false;
        bStartClear = false;
        bStartValve = false;
      }
    }
      
    // Если открыты все разрешенные клапана, то будет пуск насоса
    if(bStart && bStartValve){
      motor1.onMotor(250);
      // Проверка, что насос запустился
      if(motor1.getStatusMotor() == true){
        flStartOperation = true; // Флаг успешного пуска
        bStartValve = false;
        bStartClear = false;
      }
    }
    // Иначе сбрасываем бит старта 
    // else { 
    //   bStart = false;
    //   Serial.println("FAIL START");
    // }
  }
}