#include "xinputs.h"



//::Button------------------------------------------------------------------------------
     
Button::Button(const uint8_t pin, uint8_t logic):
        m_logic(logic),
        m_pin(pin)
{
        pinMode(m_pin, INPUT_PULLUP);
}

bool Button::read() {
        m_state = (m_logic) ? digitalRead(m_pin) : !digitalRead(m_pin);
        m_lastState = m_state;
        m_lastTime = millis();
        
        return m_state;
}

bool Button::getState() {
        return m_state; 
}


bool Button::getLastState() { 
        return m_lastState; 
}

uint32_t Button::getLastTime() { 
        return m_lastTime; 
}


//::RotaryEncoder------------------------------------------------------------------------------

RotaryEncoder::RotaryEncoder(const uint8_t clk, const uint8_t dt, const uint8_t sw):
        m_clkPin(clk),
        m_dtPin(dt),
        m_swPin(sw),
        m_button(m_swPin, false)
{
        pinMode(m_clkPin, INPUT);
        pinMode(m_dtPin, INPUT);     
}

uint8_t RotaryEncoder::getClkPin() {
        return m_clkPin;
}

void RotaryEncoder::reset() {
        m_counter = 0;
        m_lastCounter = 0;
}

void RotaryEncoder::clkHandler() {
        static uint8_t clk_state;
        static uint8_t clk_last_state = digitalRead(m_clkPin);

        clk_state = digitalRead(m_clkPin);
        
        if(clk_state != clk_last_state) {
                if(digitalRead(m_dtPin) != clk_state) {
                        m_counter++;
                }
                else {
                        m_counter--;
                }
               // Serial.println(m_counter);
        }
        clk_last_state = clk_state;        
}

int32_t RotaryEncoder::read() {

        m_lastCounter = m_counter;
        
        return m_counter;
}

int32_t RotaryEncoder::readDeltaMove() {
        int32_t delta = (m_counter - m_lastCounter);
        read();
        return delta;
}

       

//::Joystick------------------------------------------------------------------------------

Joystick::Joystick(const uint8_t xpin, const uint8_t ypin, const uint8_t swpin):
        m_xpin(xpin),
        m_ypin(ypin),
        m_button(swpin, false)
{
        pinMode(m_xpin, INPUT);
        pinMode(m_ypin, INPUT);
}

        
axe_t Joystick::readAxeControlData() {
        axe_t control_axe;
        read();
        control_axe.y = m_axe.y - m_axe.x + Inputs::JoystickMiddleX;
        control_axe.x = m_axe.y + m_axe.x - Inputs::JoystickMiddleX;
        
        control_axe.x = clamp(control_axe.x, 0, 1023);
        control_axe.y = clamp(control_axe.y, 0, 1023);
        
        control_axe.x = map(control_axe.x, 0, 1023, 0, 200);
        control_axe.y = map(control_axe.y, 0, 1023, 0, 200);

        return control_axe;
}

        
axe_t Joystick::read() {
        m_axe.x = analogRead(m_xpin);
        m_axe.y = analogRead(m_ypin);

        return m_axe;
}

axe_t Joystick::getAxeData() {
        return m_axe;
}



//::Potentiometer------------------------------------------------------------------------------

Potentiometer::Potentiometer(const uint8_t pin):
        m_pin(pin)
{
        pinMode(m_pin, INPUT);
}

uint16_t Potentiometer::read() {
        m_value = analogRead(m_pin);
        return m_value;
}
