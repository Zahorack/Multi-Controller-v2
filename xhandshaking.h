/*
 * Handshaking.h
 *
 *  Created on: 30.8.2019
 *      Author: Zahorack
 */

#ifndef HANDSHAKING_H_
#define HANDSHAKING_H_

#include "Arduino.h"
#include "Queue.h"
#include "xpacket.h"
#include "Result.h"

namespace Util {

namespace HandshakingStates {
enum Enum : uint8_t {
	Busy = 0,
	Free
};
}

class Handshaking {

	Container::Queue<Control::Packet, 5> m_packetQueue;
	uint8_t m_state;

public:
	Handshaking();

	bool add(Control::Packet);
	Container::Result<Control::Packet> update();
	bool check(Control::Packet);

};

}

#endif
