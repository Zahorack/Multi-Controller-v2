#ifndef XCOM_H_
#define XCOM_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "xinputs.h"


//typedef struct {
//        uint16_t start_mark;
//        uint32_t id;
//        uint16_t data_len;
//        uint8_t  type;
//        uint8_t  data_crc;
////        uint8_t  *data;
//} __attribute__((packed)) packetHeader_t;

typedef struct {
        uint32_t id;
        uint8_t  type;
} __attribute__((packed)) packetHeader_t;

typedef struct {
        axe_t joystickData;
} __attribute__((packed)) controlData_t;

typedef struct {
        uint32_t        uptime;
        uint16_t        battery_voltage;
} __attribute__((packed)) statusData_t;

namespace PacketType {
enum packet_type : uint8_t {
        Nack = 0,
        Ack,
        Status,
        ControlData
};
}


class Communication {
        
        SoftwareSerial *m_rf;
        uint16_t m_packetIndex;
        
        const uint16_t PacketMark = 0x4B4C;
public:
        Communication(SoftwareSerial *uart):
	        m_rf(uart)
	{
	        m_rf->begin(57600); 
	}

        uint16_t getPacketMark() {
		return PacketMark;
        }
        
        void send(uint8_t packet_type);
        void update();
        void sendControlData();
        
        uint8_t calc_crc8(uint8_t  * data, uint16_t len);
        
};






#endif
