#ifndef XINPUTS_H_
#define XINPUTS_H_

#include <Arduino.h>
#include "extern.h"

typedef struct {
        uint16_t x;
        uint16_t y;
}__attribute__((packed)) axe_t;

namespace Inputs {
        const uint16_t JoystickMiddleX = 498;
        const uint16_t JoystickMiddleY = 507;    
        const uint16_t JoystickOffset = 20; 
}

//::Button------------------------------------------------------------------------------
class Button {

        const uint8_t m_pin;
        const bool m_logic;
        
        bool m_state;
        bool m_lastState;
        
        uint32_t m_lastTime;
public:      
        Button(const uint8_t pin, uint8_t logic);
        
        bool read();

        bool getState();
        bool getLastState();
        uint32_t getLastTime();
};

//::RotaryEncoder------------------------------------------------------------------------------
class RotaryEncoder {
        
        const uint8_t m_clkPin, m_dtPin, m_swPin;
        int32_t m_counter, m_lastCounter;
public:
        RotaryEncoder(const uint8_t clk, const uint8_t dt, const uint8_t sw);
        Button m_button;
        
        uint8_t getClkPin();
        void reset();
        void clkHandler();
        int32_t read();
        int32_t readDeltaMove();
        
};

//::Joystick------------------------------------------------------------------------------
class Joystick {

        axe_t m_axe;
        const uint8_t m_xpin, m_ypin;
        
public:
        Joystick(const uint8_t xpin, const uint8_t ypin, const uint8_t swpin);
        
        Button m_button;
        
        axe_t read();
        axe_t getAxeData();
        axe_t readAxeControlData();
};


//::Potentiometer------------------------------------------------------------------------------
class Potentiometer {
        
        const uint8_t m_pin;
        uint16_t m_value;
public:
        Potentiometer(const uint8_t pin);

        uint16_t read();
};

#endif
