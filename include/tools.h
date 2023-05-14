#include <Arduino.h>
// Класс для подсчет количества объектов класса
class numcopies
{
private:
    static size_t& m_count()
    {
        static size_t c(0);
        return c;
    }
public:
    numcopies() { m_count()++; }
    ~numcopies() { m_count()--; }
    const size_t& countObjects = m_count();
};