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

uint32_t currentTime, loopTime;
bool led13 = false;
int tPWM = 0;
void setup() {
  Serial.begin(2400);
  currentTime = millis();
  loopTime = currentTime;
  // pinMode(10, OUTPUT);
}
void loop() {
  // currentTime = millis();
  if(Serial.available()>0){
    bufStr = Serial.readString();
    Serial.println(bufStr);
    // Данные с дисплея по UART
    // Считываем команды от дисплея и парсим их
    for(int i=0; i<bufStr.length(); i++){
      if(memcmp(&bufStr[i],V1PERM,sizeof(V1PERM))==0)
        valve1.setPermitionOpenValve();
      if(memcmp(&bufStr[i],V1UNPERM,sizeof(V1UNPERM))==0)
        valve1.unsetPermitionOpenValve();
      if(memcmp(&bufStr[i],V2PERM,sizeof(V2PERM))==0)
        valve2.setPermitionOpenValve();
      if(memcmp(&bufStr[i],V2UNPERM,sizeof(V2UNPERM))==0)
        valve2.unsetPermitionOpenValve();
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

  // Получено значение начать операцию и операция еще не начата
  if(bStart == true && flStartOperation == false){ 
    // Включение прерывания и расчета на расходомерах
    // если соответствующий клапан будет в работе
    if(valve1.getPermitionOpenValve()){
      Serial.println("VALVE1 READY");
      flowMeter1.onIntFlowMeter();
      flowMeter1.onFlowMeter();
    }
    if(valve2.getPermitionOpenValve()){
      Serial.println("VALVE2 READY");
      flowMeter2.onIntFlowMeter();
      flowMeter2.onFlowMeter();
    }
    if(valve3.getPermitionOpenValve()){
      Serial.println("VALVE3 READY");
      flowMeter3.onIntFlowMeter();
      flowMeter3.onFlowMeter();
    }
    if(valve4.getPermitionOpenValve()){
      Serial.println("VALVE4 READY");
      flowMeter4.onIntFlowMeter();
      flowMeter4.onFlowMeter();
    }
    // Открываем клапаны которым выдано разрешение
    valve1.openValve();
    valve2.openValve();
    valve3.openValve();
    valve4.openValve();

    if(valve1.getStatusValve())
      Serial.println("VALVE1 OPEN");
    if(valve2.getStatusValve())
      Serial.println("VALVE2 OPEN");
    if(valve3.getStatusValve())
      Serial.println("VALVE3 OPEN");
    if(valve4.getStatusValve())
      Serial.println("VALVE4 OPEN");
    
    // Если открыта хотя бы один клапан, то будет пуск насоса
    if(valve1.getStatusValve() || valve2.getStatusValve() ||
      valve3.getStatusValve() || valve4.getStatusValve()){
      flStartOperation = true; // Флаг успешного пуска
      motor1.onMotor();
    }
    // Иначе сбрасываем бит старта 
    else { 
      bStart = false;
      Serial.println("FAIL START");
    }

    if(motor1.getStatusMotor())
      Serial.println("MOTOR ON");
  }

  // ПРОМЫВКА
  if(bClearing == true){
    bClearing = false;
    flowMeter1.onIntFlowMeter();
    flowMeter1.onFlowMeter();
    flowMeter2.onIntFlowMeter();
    flowMeter2.onFlowMeter();
    flowMeter3.onIntFlowMeter();
    flowMeter3.onFlowMeter();
    flowMeter4.onIntFlowMeter();
    flowMeter4.onFlowMeter();
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

    // Выключаем расходомеры
    flowMeter1.offFlowMeter();
    flowMeter2.offFlowMeter();
    flowMeter3.offFlowMeter();
    flowMeter4.offFlowMeter();
    // Выключение прерывания на расходомерах
    flowMeter1.offIntFlowMeter();
    flowMeter2.offIntFlowMeter();
    flowMeter3.offIntFlowMeter();
    flowMeter4.offIntFlowMeter();

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
  if((flowMeter1.getError() & eMaxVolume) == true){
    valve1.closeValve();
    if(!valve1.getStatusValve())
      Serial.println("VALVE1 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 2"
  if((flowMeter2.getError() & eMaxVolume) == true){
    valve2.closeValve();
    if(!valve2.getStatusValve())
      Serial.println("VALVE2 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 3"
  if((flowMeter3.getError() & eMaxVolume) == true){
    valve3.closeValve();
    if(!valve3.getStatusValve())
      Serial.println("VALVE3 CLOSE MAX");
  }
  // Произошло событие "Сделан максимальный объем на расходомере 4"
  if((flowMeter4.getError() & eMaxVolume) == true){
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
  
  // if(millis() >= (currentTime + cTime)){
  //   currentTime = millis();
  //   tPWM++;
  //   if(tPWM > 2)
  //     tPWM = 0;
  //   // Serial.println(tPWM);
  // }
  // if(tPWM == 0)
  //   analogWrite(10, 1);
  // else if(tPWM == 1)
  //   analogWrite(10, 2);
  // else if(tPWM == 2)
  //   analogWrite(10, 3);

  // Serial.println("CIRCLE TIME: " + (String)currentTime);
    // Экран передает в ЦПУ
    // V1;1;V2;0;V3;1;V4;1;run;1;stop;0;promivka;0;
    // ЦПУ передает в экран
    // flow;0;V1;1;V2;0;V3;1;V4;1;run;0;stop;0;promivka;0;pressure;0;
    // +
    // 1010
    // a[0][0] = (flow, 0)
    // a[1][1] = (v1, 0)
    // serial.output(valve1.status, flowMeter1.getFlowRate());
}