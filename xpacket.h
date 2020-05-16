/*
 * Packet.h
 *
 *  Created on: 2.9. 2018
 *      Author: Zahorack
 */

#ifndef CONTROL_PACKET_H_
#define CONTROL_PACKET_H_

#include "Arduino.h" 
#include "xinputs.h"

namespace Control
{
        const uint16_t PacketMark = 0x4B4C;

        namespace PacketType
        {
                enum Enum : uint8_t {
                        Nack = 0,
                        Ack,
                        Status,
                        ManualControl,
                        OpenLeftFeeder,
                        OpenRightFeeder,
                        ManualCalibration,
                        SingleBeamSonarData,
                        UltrasonicData,
                        SonarData
                };
        }

//        typedef struct {
//                uint16_t x;
//                uint16_t y;
//        }__attribute__((packed)) axe_t;


        struct __attribute__((packed)) PacketHeader {
                uint32_t id;
                PacketType::Enum type;
        };


        struct __attribute__((packed)) NackPacket {
        };

        struct __attribute__((packed)) AckPacket {
        };

        struct OpenLeftFeederPacket {
        };

        struct OpenRightFeederPacket {
        };

        struct __attribute__((packed)) StatusPacket {
                uint32_t uptime;
                uint8_t batteryChargeLevel;
        };

        struct __attribute__((packed)) SingleBeamSonarDataPacket {
                uint16_t echoInterval;
        };

        struct __attribute__((packed)) UltrasonicData {
                uint16_t closestEcho;
                uint16_t strongestEcho;
        };

        struct __attribute__((packed)) SonarData {
                UltrasonicData ultrasonic[5];
        };


        struct __attribute__((packed)) ManualCalibrationPacket {
                int8_t directionCalibration;
                uint8_t maxPower;
        };
        
        struct __attribute__((packed)) ManualControlPacket {
                axe_t   joystickData;
        };

        union PacketContents {
                NackPacket nackPacket;
                AckPacket ackPacket;
                StatusPacket statusPacket;
                ManualControlPacket dataPacket;
                ManualCalibrationPacket calibrationPacket;
                SingleBeamSonarDataPacket singleBeamPacket;
                UltrasonicData ultrasonicDataPacket;
                SonarData sonarDataPacket;
        };

        using Crc = uint8_t;


        class Packet {

        public:

                PacketHeader header;
                PacketContents contents;

                static uint8_t SizeForType(PacketType::Enum packetType);
                static Crc CalculateCRC8(const uint8_t *data, uint16_t length);

                template<typename T>
                static Crc CalculateCRC8(const T &data) {
                        return Packet::CalculateCRC8(reinterpret_cast<const uint8_t *>(&data), sizeof(T));
                }

                template<PacketType::Enum Type>
                static constexpr Packet Create(uint32_t id) {
                        return {
                                {
                                        .id = id,
                                        .type = Type
                                }
                        };
                }
        };
}

#endif /* CONTROL_PACKET_H_ */
