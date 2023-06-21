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

static bool systemInitialise = false;
static bool dysplayInitialise = false;  
static bool testCommStatic = false;
static uint32_t testValue = 1;
static uint32_t prevValue = 0;

bool bStart = false;
bool bClearing = false;
bool bClearingExt = false;
bool bStop = false;
bool flStartOperation = false;
bool flEmergencyStop = false;
bool bStartClear = false;
bool bStartValve = false;
bool flButtonStartPressed, flButtonStopPressed = false;
uint32_t currentTime = 0;

Nextion myNextion(portHMI, 9600);

RelayIn * RelayIn::instances[4] = {NULL, NULL};
RelayIn buttonStart = RelayIn(buttonPinStart, INPUT_PULLUP); // Старт
RelayIn buttonStop = RelayIn(buttonPinStop, INPUT_PULLUP); // Стоп

// Выходное реле
RelayOut clearingExtLamp = RelayOut(clearExt, OUTPUT);

// Инициализация объекта расходомера с вызовом конструктора класса FlowMeter(byte ePin, bool eStatus)
// Очишаем экземпляры классов для 4 расходомеров
FlowMeter * FlowMeter::instances[4] = {NULL, NULL};
FlowMeter flowMeter1(flowSensor1Pin, false);
FlowMeter flowMeter2(flowSensor2Pin, false);
FlowMeter flowMeter3(flowSensor3Pin, false);
FlowMeter flowMeter4(flowSensor4Pin, false);
// Инициализация объекта клапана с вызовом конструктора класса Valve(byte ePin, bool eStatus)
Valve valve1(valve1Pin, false);
Valve valve2(valve2Pin, false);
Valve valve3(valve3Pin, false);
Valve valve4(valve4Pin, false);

Motor motor1(pumpPin, false);

// Прототипы функций
bool testComm(uint16_t _delay = 1000);
bool initSystem();
void messageDysplay();
void startOperation();
void stopOperation();
void clearing();
void clearingExt();
void calculate();
// Проверка срабатывания кнопок
void getRelay();

void setup() {
  Serial.begin(115200);
  myNextion.beginCom();
  myNextion.init();

  // buttonStart.onInt();
  // buttonStop.onInt();

  currentTime = millis();

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
      if(motor1.offMotor(pumpStopDelay)){
        myNextion.setComponentValue(M1,STOP);
        myNextion.setComponentValue(RUNSYS,STOP);
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
        Serial.println(flStartOperation);
    }
  }
}

bool testComm(uint16_t _delay){
  // Если таймер сработал, то проверяем поле vtest
  if (millis() >= (currentTime + _delay))
  {
    currentTime = millis();
    testValue = myNextion.getComponentValue("vtest");
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
      flowMeter1.onFullFlowMeter();
      flowMeter2.onFullFlowMeter();
      flowMeter3.onFullFlowMeter();
      flowMeter4.onFullFlowMeter();

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
      if(message == STOPHMI || message == STPCLR || message == STPCLREXT){
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
      if(message == CLEAREXT && !flStartOperation){
        bClearingExt = true;
        bStart = false;
        flStartOperation = false;
        bStop  = false;
      }
      else if(message == CLEAREXT && flStartOperation){
        myNextion.setComponentValue(CLRBTNEXT, 0);
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
bool stopButtonPressed = false;
// Проверка срабатывания кнопок
void getRelay(){
  // if(buttonStart.getInt() && !bStart && !stopButtonPressed){
  //   buttonStart.clearInt();
  //   Serial.print("START");
  //   bStart = true;
  //   bStop  = false;
  // }
  // if(buttonStop.getInt() && !bStop && !stopButtonPressed){
  //   buttonStop.clearInt();
  //   Serial.print("STOP");
  //   bStart = false;
  //   flStartOperation = false;
  //   bStop  = true;
  //   stopButtonPressed = true;
  // }
  // else if(buttonStop.getInt() && stopButtonPressed){
  //   stopButtonPressed = false;
  //   buttonStop.clearInt();
  // }
  if(!buttonStart.getCondition() && !flButtonStartPressed && !flButtonStopPressed){
    Serial.print("START");
    flButtonStartPressed = true;
    bStart = true;
    bStop  = false;
  }
  else if(buttonStart.getCondition() && flButtonStartPressed)
  {
    Serial.print("START unpress");
    flButtonStartPressed = false;
  }
  if(!buttonStop.getCondition() && !flButtonStopPressed){
    Serial.print("STOP");
    flButtonStopPressed = true;
    bStart = false;
    flStartOperation = false;
    bStop  = true;
  }
  else if(buttonStart.getCondition() && flButtonStopPressed){
    Serial.print("stop unpress");
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
      // Сброс значений на расходомерах
      flowMeter1.onFlowMeter();
      flowMeter2.onFlowMeter();
      flowMeter3.onFlowMeter();
      flowMeter4.onFlowMeter();

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
        if(valve1.openValve((uint32_t)valveOpenDelay)){
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V1,OPEN);
        #endif
        }
        if(valve2.openValve((uint32_t)valveOpenDelay)){
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V2,OPEN);
        #endif
        }
        if(valve3.openValve((uint32_t)valveOpenDelay)){
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V3,OPEN);
        #endif
        }
        if(valve4.openValve((uint32_t)valveOpenDelay)){
        #ifdef DEBUG_HMI
          myNextion.setComponentValue(V4,OPEN);
        #endif
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
      if(motor1.onMotor(pumpStartDelay)){
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
    //   Serial.println("FAIL START");
    // }
  }
}
void stopOperation(){
  if(bStop == true){
    bStop = false;
    flStartOperation = false;
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

    if(motor1.offMotor()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,STOP);
      myNextion.setComponentValue(RUNSYS, STOP);
      #endif
      // myNextion.sendCommand("page 0");
      myNextion.sendCommand("vis runProc,0");
    }
    if(clearingExtLamp.getCondition())
      clearingExtLamp.close();
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
    if(motor1.onMotor()){
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,RUN);
      #endif
    }
  }
}
void clearingExt(){
  // ПРОМЫВКА ТОЛЬКО НАСОСОМ (БАЙПАС)
  if(bClearingExt == true){
    bClearingExt = false;
    #ifdef DEBUG_HMI
    myNextion.setComponentValue(V1,valve1.getStatusValve());
    myNextion.setComponentValue(V2,valve2.getStatusValve());
    myNextion.setComponentValue(V3,valve3.getStatusValve());
    myNextion.setComponentValue(V4,valve4.getStatusValve());
    #endif
    if(motor1.onMotor()){
      clearingExtLamp.extOpen();
      #ifdef DEBUG_HMI
      myNextion.setComponentValue(M1,motor1.getStatusMotor());
      #endif
    }
  }
}
void calculate(){
  // Выполняем расчет расхода на расходомерах
  if(flowMeter1.calcRateVolume()){
    #ifdef DEBUG_HMI
    myNextion.setComponentValue(VOL1CUR,flowMeter1.getVolume());
    #endif
    // myNextion.setComponentText(VOL1CUR,(String)flowMeter1.getVolume());
  }
  if(flowMeter2.calcRateVolume()){
    #ifdef DEBUG_HMI
    myNextion.setComponentValue(VOL2CUR,flowMeter2.getVolume());
    #endif
    // myNextion.setComponentText(VOL2CUR,(String)flowMeter2.getVolume());
  }
  if(flowMeter3.calcRateVolume()){
    #ifdef DEBUG_HMI
    myNextion.setComponentValue(VOL3CUR,flowMeter3.getVolume());
    #endif
    // myNextion.setComponentText(VOL3CUR,(String)flowMeter3.getVolume());
  }
  if(flowMeter4.calcRateVolume()){
    #ifdef DEBUG_HMI
    myNextion.setComponentValue(VOL4CUR,flowMeter4.getVolume());
    #endif
    // myNextion.setComponentText(VOL4CUR,(String)flowMeter4.getVolume());
  }
}