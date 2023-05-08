#include <Arduino.h>
// Пример скетча с клапаном, который управляется и в
#include "valve.h"
#include "flow.h"
#include "motor.h"

bool bStart = false;
bool bClearing = false;
bool bStop = false;
bool flStartOperation = false;
// Инициализация объекта расходомера с вызовом конструктора класса FlowMeter(byte ePin, bool eStatus)
// Очишаем экземпляры классов для 4 расходомеров
FlowMeter * FlowMeter::instances[4] = {NULL, NULL};
// Ножки расходомеров настраивать в flow.h
FlowMeter flowMeter1(flowSensor1,false);
FlowMeter flowMeter2(flowSensor2,false);
FlowMeter flowMeter3(flowSensor3,false);
FlowMeter flowMeter4(flowSensor4,false);
// Инициализация объекта клапана с вызовом конструктора класса Valve(byte ePin, bool eStatus)
Valve valve1(13,false);
Valve valve2(14,false);
Valve valve3(15,false);
Valve valve4(16,false);

Motor motor1(10, false);
void setup() {
}

void loop() {
  // Данные с дисплея по UART
  // Тут должно быть, если передано значение "разрешения" открытия клапана с дисплея, то:
  valve1.setPermitionOpenValve();
  valve2.setPermitionOpenValve();
  valve3.setPermitionOpenValve();
  valve4.setPermitionOpenValve();
  // Иначе:
  // valve1.unsetPermitionOpenValve();
  // valve2.unsetPermitionOpenValve();
  // valve3.unsetPermitionOpenValve();
  // valve4.unsetPermitionOpenValve();

  // Получено значение начать операцию и операция еще не начата
  if(bStart == true && flStartOperation == false){
    flStartOperation = true;
    // Очистка ошибок на клапанах
    valve1.clearError();
    valve2.clearError();
    valve3.clearError();
    valve4.clearError();
    // Включение прерывания и расчета на расходомерах
    // если соответствующий клапан в работе
    if(valve1.getPermitionOpenValve()){
      flowMeter1.onIntFlowMeter();
      flowMeter1.onFlowMeter();
    }
    if(valve2.getPermitionOpenValve()){
      flowMeter2.onIntFlowMeter();
      flowMeter2.onFlowMeter();
    }
    if(valve3.getPermitionOpenValve()){
      flowMeter3.onIntFlowMeter();
      flowMeter3.onFlowMeter();
    }
    if(valve4.getPermitionOpenValve()){
      flowMeter4.onIntFlowMeter();
      flowMeter4.onFlowMeter();
    }
    // Открываем клапаны которым выдано разрешение
    valve1.openValve();
    valve2.openValve();
    valve3.openValve();
    valve4.openValve();

    motor1.onMotor();
  }
  if(bStop == true){
    // Закрываем клапаны
    valve1.closeValve();
    valve2.closeValve();
    valve3.closeValve();
    valve4.closeValve();
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
  }
  // Выполняем расчет расхода на расходомерах
  flowMeter1.calcRateVolume();
  flowMeter2.calcRateVolume();
  flowMeter3.calcRateVolume();
  flowMeter4.calcRateVolume();
  // Получаем значение расхода л/мин с расходомера 1 и отправлям его в экран
  flowMeter1.getFlowRate();
  flowMeter2.getFlowRate();
  flowMeter3.getFlowRate();
  flowMeter4.getFlowRate();
  // Получаем значение объема с расходомера 1 и отправлям его в экран
  flowMeter1.getVolume();
  flowMeter2.getVolume();
  flowMeter3.getVolume();
  flowMeter4.getVolume();

  // Произошло событие "Сделан максимальный объем на расходомере 1"
  if((flowMeter1.getError() & eMaxVolume) == true)
    valve1.closeValve();
  // Произошло событие "Сделан максимальный объем на расходомере 2"
  if((flowMeter2.getError() & eMaxVolume) == true)
    valve2.closeValve();
  // Произошло событие "Сделан максимальный объем на расходомере 3"
  if((flowMeter3.getError() & eMaxVolume) == true)
    valve3.closeValve();
  // Произошло событие "Сделан максимальный объем на расходомере 4"
  if((flowMeter4.getError() & eMaxVolume) == true)
    valve4.closeValve();            
  // выключение насоса по последнему закрывшемуся клапану
  if(!(valve1.getStatusValve() || valve2.getStatusValve() ||
      valve3.getStatusValve() || valve4.getStatusValve()))
    motor1.offMotor();
  // Добавить логику: закрытие по последнему клапану

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