#ifndef HEADER_INIT_VALUES
#define HEADER_INIT_VALUES

// Определяем задержки в миллисекундах
#define valveOpenDelay 500 // задержка при открытии клапана
#define valveCloseDelay 2000 // задержка при закрытии клапана
#define pumpStartDelay 200 // задержка при запуске насоса
#define pumpStopDelay 1000 // задержка при остановке насоса

#define maxVolumeInit 1000

#ifdef __AVR_ATmega2560__
    #define DEBUGLN(x) Serial.println(x)
    #define DEBUG(x)   Serial.print(x)
#else
    #define DEBUGLN(x)
    #define DEBUG(x)
#endif

#endif