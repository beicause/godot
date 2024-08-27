/**************************************************************************/
/*  packet_peer_kcp.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef PACKET_PEER_KCP_H
#define PACKET_PEER_KCP_H

#include "core/io/packet_peer_udp.h"
#include <ikcp.h>

class PacketPeerKCP : public PacketPeer {
	GDCLASS(PacketPeerKCP, PacketPeer);

	ikcpcb *kcp = nullptr;
	Ref<PacketPeerUDP> udp_peer = nullptr;

protected:
	static void _bind_methods();

public:
	Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	int get_available_packet_count() const override;
	int get_max_packet_size() const override;

	void init_kcp(int p_conv, int p_mode = 0);
	void update_kcp();

	int get_kcp_conv() const;
	void set_kcp_mode(int p_mode);

	void set_udp_peer(Ref<PacketPeerUDP> p_udp_peer);
	Ref<PacketPeerUDP> get_udp_peer() const;

	static Ref<PacketPeerKCP> create_from_udp(Ref<PacketPeerUDP> p_udp_peer);
	~PacketPeerKCP();
};

#endif // PACKET_PEER_KCP_H
