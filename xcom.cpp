/*
 * Comunication.cpp
 *
 *  Created on: 2.9. 2018
 *      Author: Zahorack
 */

#include "xcom.h"
#include "extern.h"
#include "xhmi.h"

namespace Control
{
        Container::Result<Packet> Communication::update()
        {
                Container::Result<Control::Packet> attempt;

                if((attempt = m_handshaking.update()).isValid) {
                        Serial.println("Handshaking send attempt\n\r");
                        send(attempt.value);
                }
                
                static uint32_t last_control_time = 0;
                if(millis() > (last_control_time + 200)) {
                        sendControlData();
                        last_control_time = millis();
                }
        
                //TRACE("rx: %d\n\r", m_rfModule.bytesAvailable());
                switch(m_state) {
                        case WaitingForNextPacket:
                                waitForNextPacket();
                                break;

                        case ReadingPacketHeader:
                                readPacketHeader();
                                break;

                        case ReadingPacketContents:
                                return readPacketContents();
                }

                return Container::Result<Packet>();
        }


        void Communication::waitForNextPacket()
        {
        
                while(m_rfModule->available() > sizeof(PacketMark)) {
                        if(readWord() == PacketMark) {
                                m_state = ReadingPacketHeader;
                                break;
                        }
                }
        }

        void Communication::readPacketHeader()
        {
                if(m_rfModule->available() >= sizeof(PacketHeader) + sizeof(Crc)) {
                       // m_rfModule->readStruct(m_currentPacket.header);
                        m_rfModule->readBytes(reinterpret_cast<uint8_t *>(&m_currentPacket.header), sizeof(m_currentPacket.header));

                        if(checkHeaderCrc()) {
                                Serial.print("<-packet type [");
                                Serial.print(m_currentPacket.header.type);
                                Serial.println("]");
                                m_state = ReadingPacketContents;
                        }
                        else {
                                m_state = WaitingForNextPacket;
                                Serial.print("Header CRC ERROR\n\r");
                        }
                }
        }

        Container::Result<Packet> Communication::readPacketContents()
        {
                if(Packet::SizeForType(m_currentPacket.header.type) > 0) {
                        if(m_rfModule->available() >= Packet::SizeForType(m_currentPacket.header.type) + sizeof(Crc)) {
                                //m_rfModule.readStruct(m_currentPacket.contents);
                                m_rfModule->readBytes(reinterpret_cast<uint8_t *>(&m_currentPacket.contents), Packet::SizeForType(m_currentPacket.header.type));

                                if(checkDataCrc()) {
                                        if(m_currentPacket.header.type != PacketType::ManualControl){
                                                sendAck();
                                        }
                                        m_state = WaitingForNextPacket;
                                        return Container::Result<Packet>(m_currentPacket);
                                }
                        }
                }
                else {
                        m_state = WaitingForNextPacket;
                        if(m_currentPacket.header.type != PacketType::Ack && m_currentPacket.header.type != PacketType::Nack) {
                                sendAck();
                        }
                        else if (m_currentPacket.header.type == Control::PacketType::Ack) {
                                m_handshaking.check(m_currentPacket);
                        }
                        return Container::Result<Packet>(m_currentPacket);
                }

                return Container::Result<Packet>();
        }

        bool Communication::checkHeaderCrc()
        {
                Crc crc = m_rfModule->read();

                if(crc != Packet::CalculateCRC8(m_currentPacket.header)) {
                        Serial.print("HEADER CRC ERROR -- RX_CRC =");
                        Serial.print(crc);
                        Serial.print("      CRC = ");
                        Serial.println(Packet::CalculateCRC8(m_currentPacket.contents.dataPacket));
                        sendNack();
                        m_state = WaitingForNextPacket;

                        return false;
                }

                return true;
        }

        bool Communication::checkDataCrc()
        {
                Crc crc = m_rfModule->read();

                if(crc != Packet::CalculateCRC8((uint8_t*)&m_currentPacket.contents,Packet::SizeForType(m_currentPacket.header.type))) {
                        Serial.print("DATA CRC ERROR -- RX_CRC =");
                        Serial.print(crc);
                        Serial.print("   CRC = ");
                        Serial.println(Packet::CalculateCRC8((uint8_t*)&m_currentPacket.contents,Packet::SizeForType(m_currentPacket.header.type)));


                        sendNack();
                        m_state = WaitingForNextPacket;

                        return false;
                }

                return true;
        }

        void Communication::sendHeader(PacketHeader header)
        {

                m_rfModule->write((uint8_t*)&PacketMark, 2);
                writeStruct(header);
                m_rfModule->write(Packet::CalculateCRC8(header));
        }

        void Communication::sendContents(PacketContents content, uint32_t size)
        {
//                writeStruct(content);
//                Serial.print("Size of content = ");
//                Serial.println(sizeof(content));
                m_rfModule->write((uint8_t*)&content, size);
                m_rfModule->write(Packet::CalculateCRC8((uint8_t*)&content, size));
//                Serial.print("RX_CRC = ");
//                Serial.println(Packet::CalculateCRC8((uint8_t*)&content, size));
        }

        void Communication::send(Packet packet)
        {
                Serial.print("-> packet type [");
                Serial.print(packet.header.type);
                Serial.println("]");
                sendHeader(packet.header);
                sendContents(packet.contents, Packet::SizeForType(packet.header.type));
        }

        void Communication::send(PacketType::Enum type)
        {
                Serial.print("-> packet type [");
                Serial.print(type);
                Serial.println("]");
                Packet packet;
                packet.header = {
                                .id = m_transmitID++,
                                .type = type
                };
                m_handshaking.add(packet);
                send(packet);
        }

        void Communication::sendAck()
        {
                Serial.print("-> packet ACK[");
                Serial.print(PacketType::Ack);
                Serial.println("]");
                PacketHeader ack = {
                                .id = m_currentPacket.header.id,
                                .type = PacketType::Ack
                };
                sendHeader(ack);
        }

        void Communication::sendNack()
        {
                Serial.print("-> packet NACK[");
                Serial.print(PacketType::Nack);
                Serial.println("]");
                PacketHeader nack = {
                                .id = m_currentPacket.header.id,
                                .type = PacketType::Nack
                };
                sendHeader(nack);
        }


        uint16_t Communication::readWord()
        {
                uint8_t buff[2];
                
                for(uint8_t n = 0; n < 2; n++) {
                        buff[n] = m_rfModule->read();
                }
                
                return (uint16_t)(buff[1]<<8 | buff[0]);
        }


        void Communication::sendControlData()
        {
                Packet control;

                control.header.id = m_transmitID++;
                control.header.type = PacketType::ManualControl;

                
                control.contents.dataPacket.joystickData = hmi.m_joystick->readAxeControlData();
                //Serial.print(control.contents.dataPacket.joystickData.x);
                //Serial.print("  ");
                //Serial.println(control.contents.dataPacket.joystickData.y);
                send(control);
        }

        void Communication::sendCalibrationData(ManualCalibrationPacket &data)
        {
                Packet calibration;
                
                calibration.header.id = m_transmitID++;
                calibration.header.type = PacketType::ManualCalibration;
                
                calibration.contents.calibrationPacket = data;
                //                Serial.print(control.contents.dataPacket.joystickData.x);
                //                Serial.print("  ");
                //                Serial.println(control.contents.dataPacket.joystickData.y);
                m_handshaking.add(calibration);
                send(calibration);
        }

        void Communication::sendStatus()
        {
                Packet status;

                status.header.id = m_transmitID++;
                status.header.type = PacketType::Status;

                //status.contents.statusPacket.batteryChargeLevel = g_batteryChargeLevel;
                status.contents.statusPacket.uptime = 0;

                send(status);

        }
}
