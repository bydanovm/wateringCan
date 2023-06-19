#ifndef HEADER_PINS
#define HEADER_PINS
    #ifdef __AVR_ATmega2560__
        #define portHMI Serial2
        // Определяем пины для управления клапанами и насосом
        #define valve1Pin 2
        #define valve2Pin 3
        #define valve3Pin 4
        #define valve4Pin 5
        #define pumpPin 6
        #define flowSensor1Pin 7
        #define flowSensor2Pin 8
        #define flowSensor3Pin 9
        #define flowSensor4Pin 10
        // Определяем пины для кнопок "Пуск" и "Стоп"
        #define buttonPinStart 11
        #define buttonPinStop 12

    // Вариант распиновки для UNO
    #elif __AVR_ATmega328P__
        #define portHMI Serial
        // Определяем пины для управления клапанами и насосом
        #define valve1Pin 2
        #define valve2Pin 3
        #define valve3Pin 4
        #define valve4Pin 5
        #define pumpPin 6
        #define flowSensor1Pin 7
        #define flowSensor2Pin 8
        #define flowSensor3Pin 13
        #define flowSensor4Pin 10
        // Определяем пины для кнопок "Пуск" и "Стоп"
        #define buttonPinStart 11
        #define buttonPinStop 12
    #endif
#endif