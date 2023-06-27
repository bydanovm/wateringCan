#ifndef HEADER_INIT_VALUES
#define HEADER_INIT_VALUES

// Определяем задержки в миллисекундах
#define valveProcessOpenDelay   200    // Задержка открытия клапана при команде СТАРТ с кнопки
#define valveProcessCloseDelay  0      // Задержка закрытия клапана при команде СТОП с кнопки
#define valveMaxLevelCloseDelay 0      // Задержка закрытия клапана при событии МАКСИМАЛЬНЫЙ УРОВЕНЬ
#define valveClearOpenDelay     200    // Задержка открытия клапана при команде ПРОМЫВКА
#define valveClearCloseDelay    0      // Задержка закрытия клапана при команде окончить ПРОМЫВКу
#define valveClearExtOpenDelay  200    // Задержка открытия клапана при команде ПРОМЫВКА ПО БАЙПАСУ
#define valveClearExtCloseDelay 0      // Задержка закрытия клапана при команде окончить ПРОМЫВКУ ПО БАЙПАСУ
#define pumpProcessOnDelay      1000   // Задержка пуска насоса при команде СТАРТ с кнопки
#define pumpProcessOffDelay     0      // Задержка останова насоса при команде СТОП с кнопки
#define pumpClearOnDelay        1000   // Задержка пуска насоса при команде ПРОМЫВКА
#define pumpClearOffDelay       0      // Задержка пуска насоса при команде окончить ПРОМЫВКу
#define pumpMaxLevelOffDelay    0      // Задержка останова насоса при событии МАКСИМАЛЬНЫЙ УРОВЕНЬ

#define maxVolumeInit 1000 // Максимальный уровень - инициализация

#define nextionSpeed 9600 // Скорость порта с HMI

#define koeffFlow1    1200     // Коэффициент расходомер 1
#define koeffFlow2    1200     // Коэффициент расходомер 2
#define koeffFlow3    1200     // Коэффициент расходомер 3
#define koeffFlow4    1200     // Коэффициент расходомер 4
#define timeLoopFlow1 50U      // Время опроса расходомера 1
#define timeLoopFlow2 50U      // Время опроса расходомера 2
#define timeLoopFlow3 50U      // Время опроса расходомера 3
#define timeLoopFlow4 50U      // Время опроса расходомера 4

#ifdef __AVR_ATmega2560__
    #define DEBUGLN(x) Serial.println(x)
    #define DEBUG(x)   Serial.print(x)
#else
    #define DEBUGLN(x)
    #define DEBUG(x)
#endif

#endif