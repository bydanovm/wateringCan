#ifndef HEADER_RELAY_IN
#define HEADER_RELAY_IN
#include <Arduino.h>
#include "pins.h"
#include "PinChangeInterrupt.h"

#define NOTPERM    0x01
#define ISOPENED   0x02
#define ISCLOSED   0x03
#define CMDOPCL    0x10 // Пришли две команды
#define ERRMODE    0x11 // Ошибка режима работы

class RelayIn {
    private:
        byte pin; // Ножка
        byte mode; // Тип работы
        bool condition; // Открыто/Закрыто
        byte errorStatus; // Байт ошибок
        static RelayIn * instances [4];
        static void inRelay1Int();
        static void inRelay2Int();
        static void inRelay3Int();
        static void inRelay4Int();
        void intRising();
        volatile bool intFlag;
    public:
        RelayIn();
        RelayIn(byte _pin, byte _mode);
        bool getCondition(void);
        byte getErrorStatus(void);
        void clearErrorStatus(void);
        void onInt();       
        void offInt();
        bool getInt();
        bool clearInt();
};

#endif