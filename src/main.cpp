#include <Arduino.h>
// Пример скетча с клапаном, который управляется и в
#include "valve.h"
#include "flow.h"

bool bStart = false;
bool bClearing = false;
bool bStop = false;
bool flStartOperation = false;
// Инициализация объекта расходомера с вызовом конструктора класса FlowMeter(byte ePin, bool eStatus)
// Может работать по аппаратному прерыванию на ножках: 2, 3, 18, 19, 20, 21
FlowMeter flowMeter1(18,false);
// Инициализация объекта клапана с вызовом конструктора класса Valve(byte ePin, bool eStatus)
Valve valve1(13,false);

// Решить проблему одна копия для всех
volatile uint16_t FlowMeter::flowFreq;

void setup() {
}

void loop() {
  // Данные с дисплея по UART
  // Тут должно быть, если передано значение "разрешения" открытия клапана с дисплея, то:
  valve1.setPermitionOpenValve();
  // Иначе:
  // valve1.unsetPermitionOpenValve();

  // Получено значение начать операцию и операция еще не начата
  if(bStart == true && flStartOperation == false){
    flStartOperation = true;
    // Очистка ошибок на клапане 1
    valve1.clearError();
    // Включение прерывания на расходомере 1
    flowMeter1.onIntFlowMeter();
    // Включаем расходомер 1
    flowMeter1.onFlowMeter();
    // Открываем клапан 1
    valve1.openValve();
  }
  if(bStop == true){
    // Закрываем клапан 1
    valve1.closeValve();
    // Выключаем расходомер 1
    flowMeter1.offFlowMeter();
    // Выключение прерывания на расходомере 1
    flowMeter1.offIntFlowMeter();
  }
  // Выполняем расчет расхода на расходомере 1
  flowMeter1.calcRateVolume();
  // Получаем значение расхода л/мин с расходомера 1 и отправлям его в экран
  flowMeter1.getFlowRate();
  // Получаем значение объема с расходомера 1 и отправлям его в экран
  flowMeter1.getVolume();

  // Произошло событие "Сделан максимальный объем на расходомере 1"
  if((flowMeter1.getError() & eMaxVolume) == true){
    bStop = true;
  }

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