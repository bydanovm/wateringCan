#ifndef HEADER_RELAY_OUT
#define HEADER_RELAY_OUT
#include <Arduino.h>

#define NOTPERM    0x01
#define ISOPENED   0x02
#define ISCLOSED   0x03
#define CMDOPCL    0x10 // Пришли две команды
#define ERRMODE    0x11 // Ошибка режима работы

class RelayOut {
    private:
        byte pin; // Ножка
        byte mode; // Тип работы
        bool condition; // Открыто/Закрыто
        bool permition; // Разрешение
        bool remoteControl; // Внешнее управление
        bool inverseMode; // Инверсивное управление
        uint8_t errorStatus; // Байт ошибок
        uint32_t currentTimeOpen; // Текущее время для открытия
        uint32_t currentTimeClose; // Текущее время для закрытия
        bool delayOpen; // Бит начала работы таймера
        bool delayClose; // Бит начала работы таймера        

    public:
        RelayOut();
        RelayOut(byte _pin, byte _mode, bool _inverseMode = false);
        bool open(void);
        bool open(uint32_t _delay);
        bool close(void);
        bool close(uint32_t _delay);
        bool extOpen(void);
        bool getCondition(void);
        void setCondition(void);
        void unsetCondition(void);
        bool getPermition(void);
        void setPermition(void);
        void unsetPermition(void);
        byte getErrorStatus(void);
        void clearErrorStatus(void);
        bool cmdOpen; // Команда открыть
        bool cmdClose; // Команда закрыть
        void onInt();
        void offInt();
        bool getInt();
};
#endif