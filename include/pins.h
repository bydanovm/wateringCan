#ifndef HEADER_PINS
#define HEADER_PINS
    #ifdef __AVR_ATmega2560__
        #define portHMI Serial2
        // Определяем пины для управления клапанами и насосом
        #define valve1Pin 2
        #define valve2Pin 3
        #define valve3Pin 4
        #define valve4Pin 5
        #define pumpPin 8
        #define flowSensor1Pin 50
        #define flowSensor2Pin 51
        #define flowSensor3Pin 52
        #define flowSensor4Pin 53
        // Определяем пины для кнопок "Пуск" и "Стоп"
        #define buttonPinStart 10
        #define buttonPinStop 11

    // Вариант распиновки для UNO
    #elif __AVR_ATmega328P__
        #define portHMI Serial
        // Определяем пины для управления клапанами и насосом
        #define valve1Pin D2
        #define valve2Pin D3
        #define valve3Pin D4
        #define valve4Pin D5
        #define pumpPin D6
        #define clearExt D13
        #define flowSensor1Pin A0 
        #define flowSensor2Pin A1
        #define flowSensor3Pin A2
        #define flowSensor4Pin A3
        // Определяем пины для кнопок "Пуск" и "Стоп"
        #define buttonPinStart D10
        #define buttonPinStop D11
    #endif
#endif