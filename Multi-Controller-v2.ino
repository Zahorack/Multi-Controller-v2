/*
 * project:   MultiController version 2
 * 
 * author:    Zahorack      
 * date:      20.7.2019
 * board:     Arduino Nano 328p
 * use:       Lakefloor mapping
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

static const uint8_t BATTERY_PIN        = A0;
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

String manualControlMenuString[5] = {"Left feeder", "Right Feeder", "Calibration", "Settings", "Back"};
Menu manualControlMenu(manualControlMenuString, 5);


namespace Menus {
enum {
        Main = 0,
        Manual,


        Size,
        Invalid
};
}
uint8_t menu_index = Menus::Main;
uint8_t menu_last_choice = 255;
uint32_t window_last_update = millis();

static const uint16_t WindowRefreshTime = 200;


HomeWindow home;
SonarWindow sonar;
static uint8_t calibration_request = false;

void setup() {
        rf.begin(57600);
        Serial.begin(9600);
        hmi.begin();
  
        
//        Timer1.initialize(50000);
//        Timer1.attachInterrupt(UART_IRQ);
        attachInterrupt(digitalPinToInterrupt(encoder.getClkPin()), encoder_handler, CHANGE);
        pinMode(LEFT_BUTTON, INPUT_PULLUP);
       
	wdt_enable(WDTO_2S);

  Serial.println("Hello");

}
/*--------------------------Main loop-------------------------------*/
void loop()
{
	wdt_reset();
        updateBattery();

        Container::Result<Control::Packet> communicationResult = com.update();
        
        if(communicationResult.isValid){
          if(communicationResult.value.header.type == Control::PacketType::Status) {
            g_boatBatteryLevel = communicationResult.value.contents.statusPacket.batteryChargeLevel;
            Serial.print("Status packet| batery: ");
            Serial.println(g_boatBatteryLevel);
          }
          if(communicationResult.value.header.type == Control::PacketType::SingleBeamSonarData) {
              Control::SingleBeamSonarDataPacket singleBeam =  communicationResult.value.contents.singleBeamPacket;
              Serial.println("SingleBeamSonarDataPacket");
              Serial.println(singleBeam.echoInterval);
              g_singleBeamEcho = singleBeam.echoInterval;
          }

        }

        
        if(menu_index == Menus::Main) {
                if(mainMenu.select()){
                        menu_last_choice = mainMenu.getChoice();
                        switch(menu_last_choice) {
                                case 0: home.update(); break;
                                case 1: menu_index = Menus::Manual; manualControlMenu.begin(); break; 
                                case 2: sonar.update(); break;
                                
                                default:  break;
                        }
                }
                else if(millis() > (window_last_update + WindowRefreshTime)){
                       switch(menu_last_choice) {
                                case 0: home.update(); break; 
                                case 2: sonar.update(); break;
                                
                                default: break;
                        } 
                        window_last_update = millis();
                }
        }
        
        else if(menu_index == Menus::Manual && manualControlMenu.select()) {
                 switch(manualControlMenu.getChoice()) {
                        case 0: com.send(Control::PacketType::OpenLeftFeeder); break;
                        case 1: com.send(Control::PacketType::OpenRightFeeder); break;
                        case 2: calibration_request = true; menu_index = Menus::Invalid; break;
                        case 4: menu_index = Menus::Main; mainMenu.begin(); break;

                        default: break;
                 }
        }

        manualCalibration(); 
}


static void manualCalibration() {

        static uint8_t first_condition = true;
        static uint32_t last_direction_correction = 0;
        static uint32_t enter_time = 0xFFFF;

        if(calibration_request){
                if(first_condition == true) {
                        hmi.m_encoder->reset();
                        enter_time = millis();
        
                        first_condition = false;
                }
                
                Control::ManualCalibrationPacket calibration;

                calibration.directionCalibration = last_direction_correction + hmi.m_encoder->read();;
                calibration.maxPower = map(hmi.m_pot->read(), 0, 1024, 50, 100);

                
                hmi.m_lcd->firstPage();
                do {
                        hmi.m_lcd->setFont(u8g_font_6x13);
                        hmi.m_lcd->setFontRefHeightText();
                        hmi.m_lcd->setFontPosTop();

                        char directionCorection[16];
                        sprintf(directionCorection, "%02d", calibration.directionCalibration);
                        hmi.m_lcd->drawStr( 40, 0, "Direction");
                        hmi.m_lcd->drawStr( 60, 15, directionCorection);

                        char powerCorrection[16];
                        sprintf(powerCorrection, "%02d", calibration.maxPower);
                        hmi.m_lcd->drawStr( 50, 30, "Power");
                        hmi.m_lcd->drawStr( 60, 45, powerCorrection);

                } while( hmi.m_lcd->nextPage() );

                if(hmi.m_encoder->m_button.read() && millis() > (enter_time + 500)) {
                        enter_time = 0xFFFF;
                        
                        first_condition = true;
                        calibration_request = false;
                        menu_index = Menus::Main;
                        last_direction_correction = calibration.directionCalibration;
                        
                        com.sendCalibrationData(calibration);
                }
        }    
}


/*
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
*/
