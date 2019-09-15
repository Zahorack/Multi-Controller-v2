/*
 * Handshaking.cpp
 *
 *  Created on: 30.8.2019
 *      Author: Zahorack
 */


#include "xhandshaking.h"

namespace Util {
        

Handshaking::Handshaking():
	m_state(HandshakingStates::Free)
{
}

bool Handshaking::add(Control::Packet packet)
{
	if(packet.header.type == m_packetQueue.value().value.header.type) {
		return false;
	}

	return m_packetQueue.enqueue(packet);
}


Container::Result<Control::Packet> Handshaking::update()
{
        static uint32_t last_time = millis();
        
	if(millis() > last_time + 200) {
                last_time = millis();
		return m_packetQueue.value();
	}

	return Container::Result<Control::Packet>();
}

bool Handshaking::check(Control::Packet ack)
{
	Container::Result<Control::Packet> current = m_packetQueue.value();

	if(ack.header.id == current.value.header.id) {
		m_packetQueue.dequeue();

		return true;
	}
	return false;
}
}
