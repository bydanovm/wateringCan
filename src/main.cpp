// #define SIM_FLOW // Симуляция расхода через ШИМ
#define DEBUG_HMI // Отладка на дисплее

#include <Arduino.h>

#include "Nextion.h"
#include "initValues.h"
#include "command.h"
#include "valve.h"
#include "flow.h"
#include "motor.h"
#include "relayIn.h"
#include "relayOut.h"

static bool systemInitialise = false; // Инициализация системы
static bool dysplayInitialise = false; // Инициализация дисплея
static bool testCommStatic = false; // Связь с дисплеем
static uint32_t testValue = 1; // Для связи с дисплеем, текущее значение
static uint32_t prevValue = 0; // Для связи с дисплеем, прошлое значение

bool bStart = false; // Команда начала тех. процесса
bool bClearing, bClearingStop = false; // Команды промывки
bool bClearingExt, bClearingExtStop = false; // Команды останова промывок
bool bStop = false; // Команда останова тех. процесса
bool flStartOperation = false; // Флаг работы тех. процесса
bool flEmergencyStop = false; // Флаг аварийного останова
bool flClearing = false; // Флаг промыки
bool flClearingExt = false; // Флаг промыки по байпасу
bool bStartClear = false; // Флаг сброса объектов при старте
bool bStartValve = false; // Флаг открытия клапанов при старте
bool flButtonStartPressed, flButtonStopPressed = false; // Флаг кнопок СТАРТ/СТОП
uint32_t currentTime, currentTime100ms = 0; // Текущее время исполнения
// Структура участия клапанов в тех. процессе
struct valvesInProcess
{
  bool valve1;
  bool valve2;
  bool valve3;
  bool valve4;
  bool res1;
  bool res2;
  bool res3;
  bool res4;
} valvesInProcess;


Nextion myNextion(portHMI, nextionSpeed);

RelayIn * RelayIn::instances[4] = {NULL, NULL};
RelayIn buttonStart = RelayIn(buttonPinStart, INPUT_PULLUP); // Старт
RelayIn buttonStop = RelayIn(buttonPinStop, INPUT_PULLUP); // Стоп

// Выходное реле
RelayOut clearingExtLamp = RelayOut(clearExt, OUTPUT);

// Инициализация объекта расходомера с вызовом конструктора класса FlowMeter(byte ePin, bool eStatus)
// Очишаем экземпляры классов для 4 расходомеров
FlowMeter * FlowMeter::instances[4] = {NULL, NULL};
// Расходомеры: ПИН, СОСТОЯНИЕ, КОЭФФИЦИЕНТ, ВРЕМЯ ОПРОСА
FlowMeter flowMeter1(flowSensor1Pin, false, koeffFlow1, timeLoopFlow1);
FlowMeter flowMeter2(flowSensor2Pin, false, koeffFlow2, timeLoopFlow2);
FlowMeter flowMeter3(flowSensor3Pin, false, koeffFlow3, timeLoopFlow3);
FlowMeter flowMeter4(flowSensor4Pin, false, koeffFlow4, timeLoopFlow4);
// Инициализация объекта клапана с вызовом конструктора класса Valve(byte ePin, bool eStatus)
Valve valve1(valve1Pin, false);
Valve valve2(valve2Pin, false);
Valve valve3(valve3Pin, false);
Valve valve4(valve4Pin, false);

Motor motor1(pumpPin, false);

// Прототипы функций
bool testComm(uint16_t _delay = 1000U);
bool initSystem(); // Инициализация
void messageDysplay(); // Сообщения от HMI
void startOperation(); //СТАРТ
void stopOperation(); // СТОП
void clearing(); // ПРОМЫВКА
void clearingExt(); // ПРОМЫВКА БАЙПАСС
void calculate(); // РАСЧЕТы
void getRelay(); // Проверка срабатывания кнопок
void countFlowPulse1();
void countFlowPulse2();
void countFlowPulse3();
void countFlowPulse4();

void setup() {
  #ifdef __AVR_ATmega2560__
  Serial.begin(115200);
  #endif
  myNextion.beginCom();
  myNextion.init();
  // Настраиваем расходомеры
  flowMeter1.beginFlowMeter(countFlowPulse1);
  flowMeter2.beginFlowMeter(countFlowPulse2);
  flowMeter3.beginFlowMeter(countFlowPulse3);
  flowMeter4.beginFlowMeter(countFlowPulse4);

  // buttonStart.onInt();
  // buttonStop.onInt();

  currentTime = millis();

  // Блок кода для симуляции расхода через ШИМ
  #if defined(SIM_FLOW)
  // ШИМ 9-10
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001100;  // x1024 fast pwm
  analogWrite(9, 127);
  #endif

  // Включение расходомера и прерывания на старте 
  delay(1000); // Задержка при пуске для выравнивания напряжения на ножках МК
}

void loop() {
  if(initSystem()){
    // Проверка срабатывания кнопок
    getRelay();
    // ПОЛУЧЕНИЕ СООБЩЕНИЙ ОТ ЭКРАНА
    messageDysplay();
    // НАЧАЛО РАБОТЫ
    startOperation();
    // СТОП
    stopOperation();
    // ПРОМЫВКА
    clearing();
    clearingExt();
    // РАСЧЕТЫ
    calculate();

    // Произошло событие "Сделан максимальный объем на расходомере 1"
    if((flowMeter1.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve1.getStatusValve()){
      if(valve1.closeValve(valveMaxLevelCloseDelay)){
        valve1.unsetPermitionOpenValve();
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(V1,CLOSE);
        #endif
        myNextion.setComponentValue(V1P,UNPERM);
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 2"
    if((flowMeter2.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve2.getStatusValve()){
      if(valve2.closeValve(valveMaxLevelCloseDelay)){
        valve2.unsetPermitionOpenValve();
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(V2,CLOSE);    
        #endif
        myNextion.setComponentValue(V2P,UNPERM);      
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 3"
    if((flowMeter3.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve3.getStatusValve()){
      if(valve3.closeValve(valveMaxLevelCloseDelay)){
        valve3.unsetPermitionOpenValve();
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(V3,CLOSE);    
        #endif
        myNextion.setComponentValue(V3P,UNPERM);
      }
    }
    // Произошло событие "Сделан максимальный объем на расходомере 4"
    if((flowMeter4.getError() & eMaxVolume) == eMaxVolume && flStartOperation && valve4.getStatusValve()){
      if(valve4.closeValve(valveMaxLevelCloseDelay)){
        valve4.unsetPermitionOpenValve();
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(V4,CLOSE);    
        #endif
        myNextion.setComponentValue(V4P,UNPERM);
      }
    }
    // Выключение насоса по последнему закрывшемуся клапану
    if(bStart == true && motor1.getStatusMotor() && 
        !(valve1.getStatusValve() || valve2.getStatusValve() ||
        valve3.getStatusValve() || valve4.getStatusValve())){
      if(motor1.offMotor(pumpMaxLevelOffDelay)){
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(M1,STOP);
        myNextion.setComponentValue(RUNSYS,STOP);
        #endif
        myNextion.sendCommand("vis runProc,0");
        bStart = false;
        flStartOperation = false;
        bStop  = true;        
      }
    }
    if (flEmergencyStop == true){
      bStart = false;
      flStartOperation = false;
      bStop  = true;
      flEmergencyStop = false;
    }
    // Проверка связи с экраном каждые 10 секунд и обновление в случае связи
    // Если связи нет, то аварийный останов
    // if(!testComm(10000)){
    //   bStart = false;
    //   flStartOperation = false;
    //   bStop  = true;
    //   dysplayInitialise = false;
    //   testCommStatic = false;
    //   stopOperation();
    // }
    if (millis() >= (currentTime + 1000)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
    {
        currentTime = millis();
        DEBUGLN(flStartOperation);
        // Отправка раз в секунду отладочных данных
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(VOL1CUR,flowMeter1.getVolume());
        myNextion.setComponentValue(VOL2CUR,flowMeter2.getVolume());
        myNextion.setComponentValue(VOL3CUR,flowMeter3.getVolume());
        myNextion.setComponentValue(VOL4CUR,flowMeter4.getVolume());
        #endif
    }
    if (millis() >= (currentTime100ms + 100)) // Рассмотреть ситуацию, когда значение будет > 4 294 967 295 (50 дней)
    {
        currentTime100ms = millis();
        DEBUGLN("Pulse1: " + (String)flowMeter1.totalFlowFreq);
        DEBUGLN("Pulse2: " + (String)flowMeter2.totalFlowFreq);
        DEBUGLN("Pulse3: " + (String)flowMeter3.totalFlowFreq);
        DEBUGLN("Pulse4: " + (String)flowMeter4.totalFlowFreq);
    }    
  }
}

bool testComm(uint16_t _delay){
  // Если таймер сработал, то проверяем поле vtest
  if (millis() >= (currentTime + _delay))
  {
    currentTime = millis();
    testValue = myNextion.getComponentValue("vtest");
    // myNextion.setComponentValue(VOL1MAX,testValue);
    // if(testValue > 0){
    if(testValue != prevValue && testValue > 1){
      prevValue = testValue;
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
      flowMeter1.onFlowMeter();
      flowMeter2.onFlowMeter();
      flowMeter3.onFlowMeter();
      flowMeter4.onFlowMeter();

      valve1.setPermitionOpenValve();
      valve2.setPermitionOpenValve();
      valve3.setPermitionOpenValve();
      valve4.setPermitionOpenValve();

      flowMeter1.setMaxVolume(maxVolumeInit);
      flowMeter2.setMaxVolume(maxVolumeInit);
      flowMeter3.setMaxVolume(maxVolumeInit);
      flowMeter4.setMaxVolume(maxVolumeInit);

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

      myNextion.setComponentValue(VOL1MAX, flowMeter1.getMaxVolume());
      myNextion.setComponentValue(VOL2MAX, flowMeter2.getMaxVolume());
      myNextion.setComponentValue(VOL3MAX, flowMeter3.getMaxVolume());
      myNextion.setComponentValue(VOL4MAX, flowMeter4.getMaxVolume());

      myNextion.sendCommand("vis runProc,0");
      
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(VOL1CUR, flowMeter1.getVolume());
      myNextion.setComponentValue(VOL2CUR, flowMeter2.getVolume());
      myNextion.setComponentValue(VOL3CUR, flowMeter3.getVolume());
      myNextion.setComponentValue(VOL4CUR, flowMeter4.getVolume());

      myNextion.setComponentValue("maxVol1",flowMeter1.getMaxVolume());
      myNextion.setComponentValue("maxVol2",flowMeter2.getMaxVolume());
      myNextion.setComponentValue("maxVol3",flowMeter3.getMaxVolume());
      myNextion.setComponentValue("maxVol4",flowMeter4.getMaxVolume());

      myNextion.setComponentValue(V1, valve1.getStatusValve());
      myNextion.setComponentValue(V2, valve2.getStatusValve());
      myNextion.setComponentValue(V3, valve3.getStatusValve());
      myNextion.setComponentValue(V4, valve4.getStatusValve());
      myNextion.setComponentValue(M1, motor1.getStatusMotor());

      myNextion.setComponentValue(CLRBTN, 0);
      myNextion.setComponentValue(CLRBTNEXT, 0);

      myNextion.setComponentValue(RUNSYS,STOP);

      myNextion.sendCommand("vis dbgLayer,0");
      #else
      myNextion.sendCommand("vis dbgLayer,1");
      #endif

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
        myNextion.setComponentValue(V1P,PERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V1UNPERM){
        valve1.unsetPermitionOpenValve();
        valve1.closeValve();
        myNextion.setComponentValue(V1P,UNPERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V1,CLOSE);
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V2PERM){
        valve2.setPermitionOpenValve();
        myNextion.setComponentValue(V2P,PERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V2UNPERM){
        valve2.unsetPermitionOpenValve();  
        valve2.closeValve();
        myNextion.setComponentValue(V2P,UNPERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V2,CLOSE);
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V3PERM){
        valve3.setPermitionOpenValve();
        myNextion.setComponentValue(V3P,PERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V3UNPERM){
        valve3.unsetPermitionOpenValve();
        valve3.closeValve();
        myNextion.setComponentValue(V3P,UNPERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V3,CLOSE);
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V4PERM){
        valve4.setPermitionOpenValve();
        myNextion.setComponentValue(V4P,PERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == V4UNPERM){
        valve4.unsetPermitionOpenValve();
        valve4.closeValve();
        myNextion.setComponentValue(V4P,UNPERM);
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V4,CLOSE);
          myNextion.setComponentValue(PRMVLV,valve1.getCountPermValve());
        #endif
      }
      if(message == STARTHMI){
        bStart = true;
        bStop  = false;
      }
      if(message == STOPHMI){
        bStart = false;
        flStartOperation = false;
        bStop  = true;
      }
      if(message == CLEAR && !flStartOperation){
        bClearing = true;
        bStart = false;
        flStartOperation = false;
        bStop  = false;
      }
      else if(message == CLEAR && flStartOperation){
        myNextion.setComponentValue(CLRBTN, 0);
      }
      if(message == STPCLR){
        bClearingStop = true;
      }
      if(message == CLEAREXT && !flStartOperation){
        bClearingExt = true;
        bStart = false;
        flStartOperation = false;
        bStop  = false;
      }
      else if(message == CLEAREXT && flStartOperation){
        myNextion.setComponentValue(CLRBTNEXT, 0);
      }
      if(message == STPCLREXT){
        bClearingExtStop = true;
      }
      if(message == UPDVOL){
        flowMeter1.setMaxVolume(myNextion.getComponentValue(VOL1MAX));
        flowMeter2.setMaxVolume(myNextion.getComponentValue(VOL2MAX));
        flowMeter3.setMaxVolume(myNextion.getComponentValue(VOL3MAX));
        flowMeter4.setMaxVolume(myNextion.getComponentValue(VOL4MAX));
        #ifdef DEBUG_HMI
        myNextion.setComponentValue("maxVol1",flowMeter1.getMaxVolume());
        myNextion.setComponentValue("maxVol2",flowMeter2.getMaxVolume());
        myNextion.setComponentValue("maxVol3",flowMeter3.getMaxVolume());
        myNextion.setComponentValue("maxVol4",flowMeter4.getMaxVolume());
        #endif
      }
      #ifdef DEBUG_HMI
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
      #endif
      message = "";
    }
}
// Проверка срабатывания кнопок
void getRelay(){

  bool flButtonStart = buttonStart.getCondition();
  if(!flButtonStart && !flButtonStartPressed && !flButtonStopPressed){
    DEBUGLN("START");
    flButtonStartPressed = true;
    bStart = true;
    bStop  = false;
  }
  else if(flButtonStart && flButtonStartPressed)
  {
    DEBUGLN("START unpress");
    flButtonStartPressed = false;
  }
  bool flButtonStop = buttonStop.getCondition();
  if(!flButtonStop && !flButtonStopPressed){
    DEBUGLN("STOP");
    // Serial.println("STOP" + (String)flButtonStop);
    flButtonStopPressed = true;
    bStart = false;
    flStartOperation = false;
    flClearing = false;
    flClearingExt = false;
    bStop  = true;
  }
  else if(flButtonStop && flButtonStopPressed){
    DEBUGLN("stop unpress");
    // Serial.println("stop unpress"  + (String)flButtonStop);
    flButtonStopPressed = false;
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

      #ifdef DEBUG_HMI
      myNextion.setComponentValue(VOL1CUR, flowMeter1.getVolume());
      myNextion.setComponentValue(VOL2CUR, flowMeter2.getVolume());
      myNextion.setComponentValue(VOL3CUR, flowMeter3.getVolume());
      myNextion.setComponentValue(VOL4CUR, flowMeter4.getVolume());
      #endif

      bStartClear = true;
    }

    // Открытие задвижек
    if(bStartValve == false && bStartClear == true){
      // Если количество разрешенных клапанов больше одного
      if(valve1.getCountPermValve() >= 1){
        // Открываем клапаны которым выдано разрешение
        if(valve1.openValve((uint32_t)valveProcessOpenDelay)){
          valvesInProcess.valve1 = true;
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V1,OPEN);
        #endif
        }
        else{
          valvesInProcess.valve1 = false;
        }
        if(valve2.openValve((uint32_t)valveProcessOpenDelay)){
          valvesInProcess.valve2 = true;
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V2,OPEN);
        #endif
        }
        else{
          valvesInProcess.valve2 = false;
        }
        if(valve3.openValve((uint32_t)valveProcessOpenDelay)){
          valvesInProcess.valve3 = true;
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V3,OPEN);
        #endif
        }
        else{
          valvesInProcess.valve3 = false;
        }
        if(valve4.openValve((uint32_t)valveProcessOpenDelay)){
          valvesInProcess.valve4 = true;
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V4,OPEN);
        #endif
        }
        else{
          valvesInProcess.valve4 = false;
        }
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
      if(motor1.onMotor(pumpProcessOnDelay)){
      #ifdef DEBUG_HMI
        myNextion.setComponentValue(M1,RUN);
      #endif
      }
      // Проверка, что насос запустился
      if(motor1.getStatusMotor() == true){
        flStartOperation = true; // Флаг успешного пуска
        bStartValve = false;
        bStartClear = false;
        #ifdef DEBUG_HMI
        myNextion.setComponentValue(RUNSYS, RUN);
        #endif
        
        // myNextion.sendCommand("page 1");
        
        myNextion.sendCommand("vis runProc,1");
      }
    }
    // Иначе сбрасываем бит старта 
    // else { 
    //   bStart = false;
    //   DEBUGLN("FAIL START");
    //   #ifdef DEBUG_HMI
    //   myNextion.setComponentValue(RUNSYS, STOP);
    //   #endif
    // }
  }
}
void stopOperation(){
  if(bStop == true){
    bStop = false;
    flStartOperation = false;
    // Закрываем клапаны
    if(valve1.closeValve(valveProcessCloseDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V1,CLOSE);
      #endif
    }
    if(valve2.closeValve(valveProcessCloseDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V2,CLOSE);
      #endif
    }
    if(valve3.closeValve(valveProcessCloseDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V3,CLOSE);
      #endif
    }
    if(valve4.closeValve(valveProcessCloseDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V4,CLOSE);
      #endif
    }

    if(motor1.offMotor(pumpProcessOffDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,STOP);
      myNextion.setComponentValue(RUNSYS, STOP);
      #endif
      // myNextion.sendCommand("page 0");
      myNextion.sendCommand("vis runProc,0");
    }

    // Устанавливаем предыдущие разрешения для клапанов при срабатывании защиты
    if(valvesInProcess.valve1 && ((flowMeter1.getError() & eMaxVolume) == eMaxVolume))
      valve1.setPrevPermitionOpenValve();
    if(valvesInProcess.valve2 && ((flowMeter2.getError() & eMaxVolume) == eMaxVolume))
      valve2.setPrevPermitionOpenValve();
    if(valvesInProcess.valve3 && ((flowMeter3.getError() & eMaxVolume) == eMaxVolume))
      valve3.setPrevPermitionOpenValve();
    if(valvesInProcess.valve4 && ((flowMeter4.getError() & eMaxVolume) == eMaxVolume))
      valve4.setPrevPermitionOpenValve();

    myNextion.setComponentValue(V1P,valve1.getPermitionOpenValve());
    myNextion.setComponentValue(V2P,valve2.getPermitionOpenValve());
    myNextion.setComponentValue(V3P,valve3.getPermitionOpenValve());
    myNextion.setComponentValue(V4P,valve4.getPermitionOpenValve());
  }
}
void clearing(){
  // ПРОМЫВКА
  if(bClearing == true  && flClearing == false){
    if(valve1.extOpenValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V1,OPEN);
      #endif
    }
    if(valve2.extOpenValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V2,OPEN);
      #endif
    }
    if(valve3.extOpenValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V3,OPEN);
      #endif
    }
    if(valve4.extOpenValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V4,OPEN);
      #endif
    }
    if(motor1.onMotor(pumpClearOnDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,RUN);
      #endif
      flClearing = true;
      bClearing = false;
    }
  }
  if(bClearingStop == true && flClearing == true){
    // Закрываем клапаны
    if(valve1.closeValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V1,CLOSE);
      #endif
    }
    if(valve2.closeValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V2,CLOSE);
      #endif
    }
    if(valve3.closeValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V3,CLOSE);
      #endif
    }
    if(valve4.closeValve()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V4,CLOSE);
      #endif
    }

    if(motor1.offMotor(pumpClearOffDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,STOP);
      myNextion.setComponentValue(RUNSYS, STOP);
      #endif
      flClearing = false;
      bClearingStop = false;
    }
  }
}
void clearingExt(){
  // ПРОМЫВКА ТОЛЬКО НАСОСОМ (БАЙПАС)
  if(bClearingExt == true && flClearingExt == false){
    if(motor1.onMotor(pumpClearOnDelay)){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(V1,valve1.getStatusValve());
      myNextion.setComponentValue(V2,valve2.getStatusValve());
      myNextion.setComponentValue(V3,valve3.getStatusValve());
      myNextion.setComponentValue(V4,valve4.getStatusValve());
      #endif
      clearingExtLamp.extOpen();
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,motor1.getStatusMotor());
      #endif
      flClearingExt = true;
      bClearingExt = false;
    }
  }
  
  if(bClearingExtStop == true && flClearingExt == true){
    if(motor1.offMotor(pumpClearOffDelay)){
      clearingExtLamp.close();
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,STOP);
      myNextion.setComponentValue(RUNSYS, STOP);
      #endif
      flClearingExt = false;
      bClearingExtStop = false;
    }
  }
}

void calculate(){
  // Выполняем расчет расхода на расходомерах
  // if(flClearing || flClearingExt || flStartOperation){
    if(flowMeter1.calcRateVolumeNew()){}
    if(flowMeter2.calcRateVolumeNew()){}
    if(flowMeter3.calcRateVolumeNew()){}
    if(flowMeter4.calcRateVolumeNew()){}
  // }
}
void countFlowPulse1(){
  flowMeter1.countFlow();
}
void countFlowPulse2(){
  flowMeter2.countFlow();
}
void countFlowPulse3(){
  flowMeter3.countFlow();
}
void countFlowPulse4(){
  flowMeter4.countFlow();
}