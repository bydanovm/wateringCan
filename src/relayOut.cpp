#include "relayOut.h"

RelayOut::RelayOut(){}
RelayOut::RelayOut(byte _pin, byte _mode, bool _inverseMode){
    pin = _pin;
    mode = _mode;
    inverseMode = _inverseMode;
    remoteControl = false;
    permition = false;
    pinMode(pin, mode);
    RelayOut::setCondition();
    digitalWrite(pin, condition);
}
bool RelayOut::open(void){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if((cmdOpen && !cmdClose) || remoteControl == true){    
            if((RelayOut::getCondition() == false && permition == true) || remoteControl == true){
                RelayOut::unsetCondition();
                digitalWrite(pin, condition);
                result = true;
                cmdOpen = false;
            }
            else if(RelayOut::getPermition() == false)
                errorStatus |= NOTPERM; 
            else if(RelayOut::getCondition() == true)
                errorStatus |= ISOPENED; 
        }
        else
            errorStatus |= CMDOPCL;
    }
    else
        errorStatus |= ERRMODE;
    return result;
}
bool RelayOut::open(uint32_t _delay){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if((RelayOut::getCondition() == false && permition == true) || remoteControl == true){
            if(delayOpen == false){
                delayOpen = true;
                currentTimeOpen = millis();
            }
            if (millis() >= (currentTimeOpen + _delay))
            {
                RelayOut::unsetCondition();
                delayOpen = false;
                result = true;
                cmdOpen = false;
            }
        }
        else if(RelayOut::getPermition() == false)
            errorStatus |= NOTPERM; 
        else if(RelayOut::getCondition() == true)
            errorStatus |= ISOPENED;  
    }
    else
        errorStatus |= ERRMODE;

    return result;
}
bool RelayOut::close(void){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if(RelayOut::getCondition() == true){
            RelayOut::setCondition();
            digitalWrite(pin, condition);
            result = true;
            remoteControl = false;
            cmdClose = false;
        } 
        else if(RelayOut::getCondition() == false)
            errorStatus |= ISCLOSED; 
    }
    else
        errorStatus |= ERRMODE;

    return result;
}
bool RelayOut::close(uint32_t _delay){
    bool result = false;
    if((mode & OUTPUT) == OUTPUT){
        if(RelayOut::getCondition() == true){
            if(delayClose == false){
                delayClose = true;
                currentTimeClose = millis();
            }
            if (millis() >= (currentTimeClose + _delay))
            {
                RelayOut::setCondition();
                digitalWrite(pin, condition);                
                delayClose = false;
                result = true;
                cmdClose = false;
            }
            remoteControl = false;
        }
        else if(RelayOut::getCondition() == false)
            errorStatus |= ISCLOSED;  
    }
    else
        errorStatus |= ERRMODE;

    return result;    
}
bool RelayOut::extOpen(void){
    bool result = false;
    remoteControl = true;
    if(open())
        result = true;
    return result;
}
bool RelayOut::getCondition(void){
    if(inverseMode)
        return !condition;
    else
        return condition;
}
void RelayOut::setCondition(void){
    if(inverseMode)
        condition = true;
    else
        condition = false;
}
void RelayOut::unsetCondition(void){
    if(inverseMode)
        condition = false;
    else
        condition = true;
}
bool RelayOut::getPermition(void){
    return permition;
}
void RelayOut::setPermition(void){
    if((mode & OUTPUT) == OUTPUT)
        permition = true;
    else
        errorStatus |= ERRMODE;
}
void RelayOut::unsetPermition(void){
    if((mode & OUTPUT) == OUTPUT)
        permition = false;
    else
        errorStatus |= ERRMODE;
}
uint8_t RelayOut::getErrorStatus(void){
    return errorStatus;
}
void RelayOut::clearErrorStatus(void){
    errorStatus = 0x00;
    cmdClose = false;
    cmdOpen = false;
    delayOpen = false;
    delayClose = false;
    remoteControl = false;
}