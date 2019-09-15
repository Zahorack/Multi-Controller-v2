#include "xhmi.h"
#include "xbitmaps.h"
#include "extern.h"

static const uint8_t DISPLAY_RESET      = A5;


//::Hmi------------------------------------------------------------------------------
void Hmi::intro()
{
        m_lcd->firstPage();  
        do {
                m_lcd->setFont(u8g_font_timB14);
                m_lcd->drawStr( 40, 22, "Multi");
                m_lcd->drawStr( 25, 40, "controller");

                m_lcd->setFont(u8g_font_6x13);
                m_lcd->drawStr( 32, 60, "version 2.0");
        } while( m_lcd->nextPage() );
}

void Hmi::begin(){
        pinMode(DISPLAY_RESET, OUTPUT);
        digitalWrite(DISPLAY_RESET, HIGH);
        intro();
}

//::Menu------------------------------------------------------------------------------
Menu::Menu(String *data, uint8_t size):
        m_list(data),
        m_size(size)
{
        hmi.m_encoder->reset();
}

uint8_t Menu::select() {
        
        static int8_t last_choice = 0;
        static int32_t counter = 0;
        static int32_t last_counter = 0;
        static bool last_button_state = LOW;

        counter = hmi.m_encoder->read();
        m_choice = counter + last_counter;

        if(m_choice >= m_size) {
                m_choice = 0;
                hmi.m_encoder->reset();
                last_counter = 0;
        }
        if(m_choice < 0) {
                m_choice = m_size-1;
                last_counter = m_size-1;
                hmi.m_encoder->reset();
        }

        if (last_choice != m_choice ||start_condition == true) {
                start_condition = false;
                hmi.m_lcd->firstPage();
                do  {
                        show();
                } while( hmi.m_lcd->nextPage() );
                last_choice = m_choice;  
        }

        if(hmi.m_encoder->m_button.read() && last_button_state == LOW) {
                last_button_state = HIGH;
                return MenuList::State::Done;
        }
        else if(!hmi.m_encoder->m_button.read()) {
                last_button_state = LOW;
                return MenuList::State::Busy;
        }       

        return MenuList::State::Busy;
}

void Menu::begin() {
        start_condition = true;
}

 void Menu::show() {
        uint8_t i, h;
        u8g_uint_t w, d;
        
        hmi.m_lcd->setFont(u8g_font_6x13);
        hmi.m_lcd->setFontRefHeightText();
        hmi.m_lcd->setFontPosTop();
        
        h = hmi.m_lcd->getFontAscent() - hmi.m_lcd->getFontDescent();
        w = hmi.m_lcd->getWidth();
        
        for( i = 0; i < m_size; i++ ) {
                char item[m_list[i].length()+1];
                m_list[i].toCharArray(item, m_list[i].length()+1);
                
                d = (w - hmi.m_lcd->getStrWidth(item))/2;
                hmi.m_lcd->setDefaultForegroundColor();
                
                if ( i == m_choice ) {
                        hmi.m_lcd->drawBox(0, i*h+1, w, h);
                        hmi.m_lcd->setDefaultBackgroundColor();
                }
                hmi.m_lcd->drawStr(d, i*h, item);
        }
}  


uint8_t Menu::getChoice() {
        return m_choice;
}

//::Windows------------------------------------------------------------------------------

void HomeWindow::show() {
        hmi.m_lcd->setFont(u8g_font_6x13);
        hmi.m_lcd->setFontRefHeightText();
        hmi.m_lcd->setFontPosTop();

        hmi.m_lcd->drawFrame(10,12,30,20);

        
        char buf[16];
        sprintf(buf, "%02d", getBatteryLevel());
        
        hmi.m_lcd->drawBitmapP( 90, 0, 1, 8, battery_bitmap);
        hmi.m_lcd->drawStr( 100, 0, buf);
        hmi.m_lcd->drawStr( 114, 0, "%");

        sprintf(buf, "%02d", g_boatBatteryLevel);
        hmi.m_lcd->drawStr( 0, 0, "boat");
        hmi.m_lcd->drawBitmapP( 25, 0, 1, 8, battery_bitmap);
        hmi.m_lcd->drawStr( 35, 0, buf);
        hmi.m_lcd->drawStr( 49, 0, "%");
}

void HomeWindow::update() {
        hmi.m_lcd->firstPage();
        do  {
                show();
        } while( hmi.m_lcd->nextPage() );
}
