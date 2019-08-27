#ifndef XHMI_H_
#define XHMI_H_

//#include <Arduino.h>
#include "U8glib.h"
#include "xinputs.h"


namespace MenuList {
        enum Move : uint8_t {
                None = 0,
                Up,
                Down,
                Right,
                Left,
                Select      
        };

         enum State : uint8_t {
                Busy = 0,
                Done           
        };     
}

//::Hmi------------------------------------------------------------------------------
class Hmi {
        void intro();
public:
        Hmi(U8GLIB_SSD1309_128X64 *scr, Button *LB, Button *RB, Button *TB, Button *LS, Button *RS, Joystick *joy, Potentiometer *pot, RotaryEncoder * enc):
                m_leftButton(LB),
                m_rightButton(RB),
                m_topButton(TB),
                m_leftSwitch(LS),
                m_rightSwitch(RS),
                m_joystick(joy),
                m_pot(pot),
                m_encoder(enc)
        {
                m_lcd = scr;
        }

        U8GLIB_SSD1309_128X64 *m_lcd;
        Button *m_leftButton, *m_rightButton, *m_topButton, *m_leftSwitch, *m_rightSwitch;
        Joystick *m_joystick;
        Potentiometer *m_pot;
        RotaryEncoder *m_encoder;
        
        void begin();



};

extern Hmi hmi;


//::Menu------------------------------------------------------------------------------
class Menu {
        String *m_list;
        const uint8_t m_size;
        int8_t m_choice;
        uint8_t m_state;

        uint8_t start_condition = true;
        
public:
        Menu(String *data, uint8_t size);

        uint8_t getChoice();
        uint8_t select();
        void show();  
        void begin();
};

//::Windows------------------------------------------------------------------------------
class Window {
        
public:    
        void update() {
                hmi.m_lcd->firstPage();
                do  {
                        
                } while( hmi.m_lcd->nextPage() );
        }
};


class HomeWindow : public Window {

public: 
        void show();
        void update();
        
};

#endif
