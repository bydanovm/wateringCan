#include <Arduino.h>
#include <tools.h>
// Создаем класс клапана
class Valve : public numcopies {
    // Задаем переменные и функции, которые будут доступны внутри класса (см. Инкапсуляция)
    private:
        byte pinValve; // Ножка клапана
        bool statusValve; // Текущий статус клапана (О/З)
        bool permitionOpenValve; // Разрешение на открытие клапана
        bool remoteControl; // Внешнее управление
        byte errorValve; // Байт ошибок и статусов клапана
        uint32_t currentTime; // Текущее время
        bool bDelay; // Бит начала работы таймера    
        // Статика    
        static uint8_t staticCountPermValve; // Переменная количества разрешенных клапанов
        static uint8_t staticCountOpenValve; // Переменная количества открытых клапанов
        static void staticCountPermValveIncr();
        static void staticCountPermValveDecr();
        static void staticCountOpenValveIncr();
        static void staticCountOpenValveDecr();
    // Задаем переменные и функции, которые будут доступны вне класса
    public:
        // Создаем конструкторы класса, для начальной установки
        // Если создается пустой класс, то по умолчанию задается ножка 10 на выход
        Valve();
        // Если создается класс с "надстройкой", то меняем значения по умолчанию на переданные
        Valve(byte ePin, bool eStatus);
        // Функция открытия клапана
        bool openValve();
        // Функция открытия клапана
        bool openValve(uint32_t);
        // Функция закрытия клапана
        bool closeValve();
        // Экстренное открытие клапана
        bool extOpenValve();
        // Функция получения состояния клапана
        bool getStatusValve();
        // Функция получения разрешения на открытие
        bool getPermitionOpenValve();
        // Функция установки разрешения на открытие
        void setPermitionOpenValve();
        // Функция снятия разрешения на открытие
        void unsetPermitionOpenValve();
        // Функция получения ошибки
        byte getError();
        // Функция очистки ошибок
        void clearError();
        // Функция получения количества клапанов разрешенных для открытия
        uint8_t getCountPermValve();
        // Функция получения количества открытых клапанов
        uint8_t getCountOpenValve();
};