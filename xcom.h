/*
 * Comunication.h
 *
 *  Created on: 2.9. 2018
 *      Author: Zahorack
 */

#ifndef UTIL_COMMUNICATION_H_
#define UTIL_COMMUNICATION_H_

#include "xpacket.h"
#include "Result.h"
//#include "Arduino.h"
#include <SoftwareSerial.h>
#include "xhandshaking.h"


        
namespace Control
{

        
        class Communication {
                SoftwareSerial *m_rfModule;
                Util::Handshaking m_handshaking;
                uint32_t m_transmitID;
                uint32_t m_receiveID;

                enum State {
                        WaitingForNextPacket,
                        ReadingPacketHeader,
                        ReadingPacketContents
                };

                State m_state = WaitingForNextPacket;
                Packet m_currentPacket;
                Packet m_transmitPacket;

        public:
                Communication(SoftwareSerial *uart):
                        m_rfModule(uart)
                {
                        m_rfModule->begin(57600); 
                }

                Container::Result<Packet> update();
                void sendStatus();
                void sendAck();
                void sendNack();
                void sendControlData();
                void sendCalibrationData(ManualCalibrationPacket &data);
                void send(PacketType::Enum type);

                void send(Packet);

        private:
                void waitForNextPacket();
                void readPacketHeader();
                uint16_t readWord();
                Container::Result<Packet> readPacketContents();
                bool checkHeaderCrc();
                bool checkDataCrc();

                template<typename T>
                void writeStruct(const T &buffer) {
                        m_rfModule->write(reinterpret_cast<const uint8_t *>(&buffer), sizeof(T));
                }

                void sendHeader(PacketHeader header);
                void sendContents(PacketContents content, uint32_t);
        };
}
#endif /* UTIL_PACKET_H_ */
