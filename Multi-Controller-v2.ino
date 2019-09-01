/*
 * project:   MultiController version 2
 * 
 * author:    Zahorack      
 * date:      20.7.2019
 * board:     Arduino Nano 328p
 * use:       Lakefloor mapping
 */

//TODO:
/*
 * Implement multiprotocol
 * 
*/

//#include "U8glib.h"
#include <TimerOne.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include "xinputs.h"
#include "xcom.h"
#include "xhmi.h"
#include "extern.h"
#include "xcom.h"
#include "xpacket.h"


/*GPIO interpretation*/
static const uint8_t LEFT_SWITCH        = 5;
static const uint8_t RIGHT_SWITCH       = 4;
static const uint8_t LEFT_BUTTON        = 9;
static const uint8_t RIGHT_BUTTON       = 6;
static const uint8_t TOP_BUTTON         = 2;
static const uint8_t JOYSTICK_BUTTON    = A2;
static const uint8_t RF_TX              = 7;
static const uint8_t RF_RX              = 8;

static const uint8_t DISPLAY_SCK        = 13;
static const uint8_t DISPLAY_MOSI       = 11;
static const uint8_t DISPLAY_CS         = 10;
static const uint8_t DISPLAY_DC         = 12;

const uint8_t        BATTERY_PIN        = A0;
static const uint8_t POTENCIOMETER      = A1;
static const uint8_t JOYSTICK_X         = A7;
static const uint8_t JOYSTICK_Y         = A6;

static const uint8_t ENCODER_CLK        = 3;
static const uint8_t ENCODER_DT         = A4;
static const uint8_t ENCODER_SW         = A3;

Button leftButton(LEFT_BUTTON, true);
Button rightButton(RIGHT_BUTTON, true);
Button topButton(TOP_BUTTON, false);
Button leftSwitch(LEFT_SWITCH, false);
Button rightSwitch(RIGHT_SWITCH, false);

Joystick joystick(JOYSTICK_X, JOYSTICK_Y, JOYSTICK_BUTTON);
Potentiometer pot(POTENCIOMETER);
RotaryEncoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);

SoftwareSerial          rf(RF_RX, RF_TX);
U8GLIB_SSD1309_128X64   lcd(DISPLAY_SCK, DISPLAY_MOSI, DISPLAY_CS, DISPLAY_DC); 

Hmi hmi(&lcd, &leftButton, &rightButton, &topButton, &leftSwitch, &rightSwitch, &joystick, &pot, &encoder);

Control::Communication com(&rf);


enum modes : uint8_t {
        rover_mode= 0,
        sunTracker_mode,
        printing_mode,
        simulation_mode,
        
        mode_size
};
uint8_t   mode = 0;


static void encoder_handler() {
        encoder.clkHandler();
}

String mainMenuString[5] = {"Home", "Manual control", "Sonar", "Chart", "Settings"};
Menu mainMenu(mainMenuString, 5);

String manualControlMenuString[4] = {"Left feeder", "Right Feeder", "Settings", "Back"};
Menu manualControlMenu(manualControlMenuString, 4);


namespace Menus {
enum {
        Main = 0,
        Manual,
        
        Size
};
}
uint8_t menu_index = Menus::Main;


HomeWindow home;

void setup() {
        rf.begin(57600);
        Serial.begin(9600);
        hmi.begin();

        
//        Timer1.initialize(50000);
//        Timer1.attachInterrupt(UART_IRQ);
        attachInterrupt(digitalPinToInterrupt(encoder.getClkPin()), encoder_handler, CHANGE);
        pinMode(LEFT_BUTTON, INPUT_PULLUP);
       

		wdt_enable(WDTO_2S);

}
/*--------------------------Main loop-------------------------------*/
void loop()
{
	wdt_reset();
        updateBattery();

        Container::Result<Control::Packet> communicationResult = com.update();

        if(communicationResult.isValid && communicationResult.value.header.type == Control::PacketType::Status) {
                Serial.print("Status packet| batery: ");
                Serial.println(communicationResult.value.contents.statusPacket.batteryChargeLevel);
        }


        
        if(menu_index == Menus::Main && mainMenu.select()) {
               switch(mainMenu.getChoice()) {
                        case 0: home.update(); break;
                        case 1: menu_index = Menus::Manual; manualControlMenu.begin(); break; 
                        
                        default: break;
               }
        }
        else if(menu_index == Menus::Manual && manualControlMenu.select()) {
                 switch(manualControlMenu.getChoice()) {
                        case 0: com.send(Control::PacketType::OpenLeftFeeder); break;
                        case 1: com.send(Control::PacketType::OpenRightFeeder); break;
                        case 3: menu_index = Menus::Main; mainMenu.begin(); break;

                        default: break;
                 }
        }
        
//Serial.print(rightButton.read());
//Serial.print("  ");
//Serial.print(leftButton.read());
//Serial.print("  ");
//Serial.print(leftSwitch.read());
//Serial.print("  ");
//Serial.print(rightSwitch.read());
//Serial.print("  ");
//Serial.print(topButton.read());
//Serial.print("  ");
//Serial.print(joystick.m_button.read());
//Serial.print("  ");
//Serial.print(pot.read());
//Serial.print("  ");
//Serial.print(joystick.read().x);
//Serial.print(" ");
//Serial.print(joystick.read().y);
//Serial.print(" ");
//Serial.print(encoder.m_button.read());
//Serial.println(" ");
}
