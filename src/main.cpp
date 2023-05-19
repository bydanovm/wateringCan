#define SIM_FLOW // Симуляция расхода через ШИМ

#include <Arduino.h>

#include "Nextion.h"
#include "command.h"
#include "valve.h"
#include "flow.h"
#include "motor.h"

static bool systemInitialise = false;
static bool dysplayInitialise = false;
static bool testCommStatic = false;
// static uint32_t prevTestValue = 0;
static uint32_t testValue = 0;
bool bStart = false;
bool bClearing = false;
bool bStop = false;
bool flStartOperation = false;
bool flEmergencyStop = false;
bool bStartClear = false;
bool bStartValve = false;
uint32_t currentTime = 0;
// String bufStr;

// String message;
char intStr[10]; // Для преобразований строк в число

Nextion myNextion(Serial, 115200);
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
Valve valve4(11, false);

Motor motor1(13, false);

// Прототипы функций
bool testComm(uint16_t _delay = 1000);
bool initSystem();
void messageDysplay();
void startOperation();
void stopOperation();
void clearing();
void calculate();

void setup() {
  // Serial.begin(115200);
  myNextion.beginCom();
  myNextion.init();

  // pinMode(13, OUTPUT);
  // digitalWrite(13,1);

  currentTime = millis();
  // loopTime = currentTime;

  // Блок кода для симуляции расхода через ШИМ
  #if defined(SIM_FLOW)
  // ШИМ 9-10
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001010;  // x8 fast pwm
  analogWrite(9, 127);
  #endif

  // Включение расходомера и прерывания на старте 
  delay(1000); // Задержка при пуске для выравнивания напряжения на ножках МК
}

void loop() {
  if(initSystem()){
    // ПОЛУЧЕНИЕ СООБЩЕНИЙ ОТ ЭКРАНА
    messageDysplay();
    // НАЧАЛО РАБОТЫ
    startOperation();
    // СТОП
    stopOperation();
    // ПРОМЫВКА
    clearing();
    // РАСЧЕТЫ
    calculate();

    // Произошло событие "Сделан максимальный объем на расходомере 1"
    if((flowMeter1.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve1.getStatusValve()){
      if(valve1.closeValve()){
        valve1.unsetPermitionOpenValve();
        myNextion.setComponentValue(V1,CLOSE);
        myNextion.setComponentValue(V1P,UNPERM);
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 2"
    if((flowMeter2.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve2.getStatusValve()){
      if(valve2.closeValve()){
        valve2.unsetPermitionOpenValve();
        myNextion.setComponentValue(V2,CLOSE);    
        myNextion.setComponentValue(V2P,UNPERM);      
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 3"
    if((flowMeter3.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve3.getStatusValve()){
      if(valve3.closeValve()){
        valve3.unsetPermitionOpenValve();
        myNextion.setComponentValue(V3,CLOSE);    
        myNextion.setComponentValue(V3P,UNPERM);
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 4"
    if((flowMeter4.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve4.getStatusValve()){
      if(valve4.closeValve()){
        valve4.unsetPermitionOpenValve();
        myNextion.setComponentValue(V4,CLOSE);    
        myNextion.setComponentValue(V4P,UNPERM);
      }
    }
    // Выключение насоса по последнему закрывшемуся клапану
    if(bStart == true && motor1.getStatusMotor() && 
        !(valve1.getStatusValve() || valve2.getStatusValve() ||
        valve3.getStatusValve() || valve4.getStatusValve())){
      if(motor1.offMotor(1000))
        myNextion.setComponentValue(M1,STOPM);
    }
    if (flEmergencyStop == true){
      bStart = false;
      flStartOperation = false;
      bStop  = true;
      flEmergencyStop = false;
    }
    // Проверка связи с экраном каждые 5 секунд и обновление в случае связи
    // Если связи нет, то аварийный останов
    if(!testComm(5000)){
      bStart = false;
      flStartOperation = false;
      bStop  = true;
      dysplayInitialise = false;
      stopOperation();
    }    
  }
}

bool testComm(uint16_t _delay){
  // Если таймер сработал, то проверяем поле vtest
  if (millis() >= (currentTime + _delay))
  {
    currentTime = millis();
    testValue = myNextion.getComponentValue("vtest");
    if(testValue > 0){
      testCommStatic = true;
    }
    else
      testCommStatic = false;
  }
  // Иначе возвращаем старое значение
  else
    return testCommStatic;
}
// Инициализация
bool initSystem(){
  if(!systemInitialise){
      flowMeter1.onFullFlowMeter();
      flowMeter2.onFullFlowMeter();
      flowMeter3.onFullFlowMeter();
      flowMeter4.onFullFlowMeter();

      valve1.setPermitionOpenValve();
      valve2.setPermitionOpenValve();
      valve3.setPermitionOpenValve();
      valve4.setPermitionOpenValve();

      flowMeter1.setMaxVolume(1000);
      flowMeter2.setMaxVolume(1000);
      flowMeter3.setMaxVolume(1000);
      flowMeter4.setMaxVolume(1000);

      systemInitialise = true;
  }
  if(!dysplayInitialise){
    if(testComm()){
      if(valve1.getPermitionOpenValve())
        myNextion.setComponentValue(V1P, PERM);
      else
        myNextion.setComponentValue(V1P, UNPERM);
      if(valve2.getPermitionOpenValve())
        myNextion.setComponentValue(V2P, PERM);
      else
        myNextion.setComponentValue(V2P, UNPERM);
      if(valve3.getPermitionOpenValve())
        myNextion.setComponentValue(V3P, PERM);
      else
        myNextion.setComponentValue(V3P, UNPERM);
      if(valve4.getPermitionOpenValve())
        myNextion.setComponentValue(V4P, PERM);
      else
        myNextion.setComponentValue(V4P, UNPERM);

      myNextion.setComponentValue(VOL1CUR, flowMeter1.getVolume());
      myNextion.setComponentValue(VOL2CUR, flowMeter2.getVolume());
      myNextion.setComponentValue(VOL3CUR, flowMeter3.getVolume());
      myNextion.setComponentValue(VOL4CUR, flowMeter4.getVolume());

      myNextion.setComponentValue(VOL1MAX, flowMeter1.getMaxVolume());
      myNextion.setComponentValue(VOL2MAX, flowMeter2.getMaxVolume());
      myNextion.setComponentValue(VOL3MAX, flowMeter3.getMaxVolume());
      myNextion.setComponentValue(VOL4MAX, flowMeter4.getMaxVolume());

      myNextion.setComponentValue(V1, valve1.getStatusValve());
      myNextion.setComponentValue(V2, valve2.getStatusValve());
      myNextion.setComponentValue(V3, valve3.getStatusValve());
      myNextion.setComponentValue(V4, valve4.getStatusValve());
      myNextion.setComponentValue(M1, motor1.getStatusMotor());

      myNextion.setComponentValue(STRBTN, 0);
      myNextion.setComponentValue(STPBTN, 0);
      myNextion.setComponentValue(CLRBTN, 0);

      dysplayInitialise = true;
    }
    else 
      dysplayInitialise = false;
  }
  return (systemInitialise & dysplayInitialise);
}

void messageDysplay(){
    String message = myNextion.listen();
    if(message != ""){
      if(message == V1PERM){
        valve1.setPermitionOpenValve();
        myNextion.setComponentValue(V1,OPEN);
      }
      if(message == V1UNPERM){
        valve1.unsetPermitionOpenValve();
        valve1.closeValve();
        myNextion.setComponentValue(V1,CLOSE);
      }
      if(message == V2PERM){
        valve2.setPermitionOpenValve();
        myNextion.setComponentValue(V2,OPEN);
      }
      if(message == V2UNPERM){
        valve2.unsetPermitionOpenValve();  
        valve2.closeValve();
        myNextion.setComponentValue(V2,CLOSE);
      }
      if(message == V3PERM){
        valve3.setPermitionOpenValve();
        myNextion.setComponentValue(V3,OPEN);
      }
      if(message == V3UNPERM){
        valve3.unsetPermitionOpenValve();
        valve3.closeValve();
        myNextion.setComponentValue(V3,CLOSE);
      }
      if(message == V4PERM){
        valve4.setPermitionOpenValve();
        myNextion.setComponentValue(V4,OPEN);
      }
      if(message == V4UNPERM){
        valve4.unsetPermitionOpenValve();
        valve4.closeValve();
        myNextion.setComponentValue(V4,CLOSE);
      }
      if(message == START){
        bStart = true;
        bStop  = false;
      }
      if(message == STOP){
        bStart = false;
        flStartOperation = false;
        bStop  = true;
      }
      if(message == CLEAR){
        bClearing = true;
        bStart = false;
        flStartOperation = false;
        bStop  = false;
      }
      if(message == UPDVOL){
        flowMeter1.setMaxVolume(myNextion.getComponentValue(VOL1MAX));
        flowMeter2.setMaxVolume(myNextion.getComponentValue(VOL2MAX));
        flowMeter3.setMaxVolume(myNextion.getComponentValue(VOL3MAX));
        flowMeter4.setMaxVolume(myNextion.getComponentValue(VOL4MAX));

        // Для проверки полученного числа
        // itoa(flowMeter1.getMaxVolume(),intStr,10);
        // String temp = String(intStr);
        // myNextion.setComponentText("t8",temp);
      }
      if(message == V1ON){
        if(valve1.openValve())
          myNextion.setComponentValue(V1,OPEN);
        else
          myNextion.setComponentValue(V1,CLOSE);
      }
      if(message == V2ON){
        if(valve2.openValve())
          myNextion.setComponentValue(V2,OPEN);
        else
          myNextion.setComponentValue(V2,CLOSE);
      }
      if(message == V3ON){
        if(valve3.openValve())
          myNextion.setComponentValue(V3,OPEN);
        else
          myNextion.setComponentValue(V3,CLOSE);
      }
      if(message == V4ON){
        if(valve4.openValve())
          myNextion.setComponentValue(V4,OPEN);
        else
          myNextion.setComponentValue(V4,CLOSE);
      }
      if(message == V1OFF){
        if(valve1.closeValve())
          myNextion.setComponentValue(V1,CLOSE);
        else
          myNextion.setComponentValue(V1,OPEN);
      }
      if(message == V2OFF){
        if(valve2.closeValve())
          myNextion.setComponentValue(V2,CLOSE);
        else
          myNextion.setComponentValue(V2,OPEN);
      }
      if(message == V3OFF){
        if(valve3.closeValve())
          myNextion.setComponentValue(V3,CLOSE);
        else
          myNextion.setComponentValue(V3,OPEN);
      }
      if(message == V4OFF){
        if(valve4.closeValve())
          myNextion.setComponentValue(V4,CLOSE);
        else
          myNextion.setComponentValue(V4,OPEN);
      }
      message = "";
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
      
      // if(valve1.getPermitionOpenValve())
      //   Serial.println("VALVE1 READY");
      // if(valve2.getPermitionOpenValve())
      //   Serial.println("VALVE2 READY");
      // if(valve3.getPermitionOpenValve())
      //   Serial.println("VALVE3 READY");
      // if(valve4.getPermitionOpenValve())
      //   Serial.println("VALVE4 READY");
      bStartClear = true;
    }

    // Открытие задвижек
    if(bStartValve == false){
      // Если количество разрешенных клапанов больше одного
      if(valve1.getCountPermValve() >= 1){
        // Открываем клапаны которым выдано разрешение
        if(valve1.openValve((uint32_t)250))
          myNextion.setComponentValue(V1,OPEN);
        if(valve2.openValve((uint32_t)500))
          myNextion.setComponentValue(V2,OPEN);
        if(valve3.openValve((uint32_t)750))
          myNextion.setComponentValue(V3,OPEN);
        if(valve4.openValve((uint32_t)1000))
          myNextion.setComponentValue(V4,OPEN);
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
      if(motor1.onMotor(250))
        myNextion.setComponentValue(M1,RUNM);
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
void stopOperation(){
  if(bStop == true){
    bStop = false;
    flStartOperation = false;
    // Закрываем клапаны
    if(valve1.closeValve())
      myNextion.setComponentValue(V1,CLOSE);
    if(valve2.closeValve())
      myNextion.setComponentValue(V2,CLOSE);
    if(valve3.closeValve())
      myNextion.setComponentValue(V3,CLOSE);
    if(valve4.closeValve())
      myNextion.setComponentValue(V4,CLOSE);

    // if(!valve1.getStatusValve())
    //   Serial.println("VALVE1 CLOSE");
    // if(!valve2.getStatusValve())
    //   Serial.println("VALVE2 CLOSE");
    // if(!valve3.getStatusValve())
    //   Serial.println("VALVE3 CLOSE");
    // if(!valve4.getStatusValve())
    //   Serial.println("VALVE4 CLOSE");

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

    if(motor1.offMotor())
      myNextion.setComponentValue(M1,STOPM);
  }
}
void clearing(){
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
    if(valve1.extOpenValve())
      myNextion.setComponentValue(V1,OPEN);
    if(valve2.extOpenValve())
      myNextion.setComponentValue(V2,OPEN);
    if(valve3.extOpenValve())
      myNextion.setComponentValue(V3,OPEN);
    if(valve4.extOpenValve())
      myNextion.setComponentValue(V4,OPEN);
    if(motor1.onMotor())
      myNextion.setComponentValue(M1,RUNM);
  }
}
void calculate(){
  // Выполняем расчет расхода на расходомерах
  if(flowMeter1.calcRateVolume()){
    myNextion.setComponentValue(VOL1CUR,flowMeter1.getVolume());
    // myNextion.setComponentText(VOL1CUR,(String)flowMeter1.getVolume());
  }
  if(flowMeter2.calcRateVolume()){
    myNextion.setComponentValue(VOL2CUR,flowMeter2.getVolume());
    // myNextion.setComponentText(VOL2CUR,(String)flowMeter2.getVolume());
  }
  if(flowMeter3.calcRateVolume()){
    myNextion.setComponentValue(VOL3CUR,flowMeter3.getVolume());
    // myNextion.setComponentText(VOL3CUR,(String)flowMeter3.getVolume());
  }
  if(flowMeter4.calcRateVolume()){
    myNextion.setComponentValue(VOL4CUR,flowMeter4.getVolume());
    // myNextion.setComponentText(VOL4CUR,(String)flowMeter4.getVolume());
  }
}