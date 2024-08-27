/**************************************************************************/
/*  packet_peer_kcp.cpp                                                   */
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

#include "packet_peer_kcp.h"
#include "core/os/time.h"

static int kcp_output_callback(const char *buf, int len, ikcpcb *kcp, void *user) {
	PacketPeerKCP *peer = (PacketPeerKCP *)user;
	ERR_FAIL_COND_V(peer->get_udp_peer().is_null(), -1);
	Error err = peer->get_udp_peer()->put_packet(reinterpret_cast<const uint8_t *>(buf), len);
	return err == OK ? 0 : -1;
}

void PacketPeerKCP::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_kcp", "p_conv", "p_mode"), &PacketPeerKCP::init_kcp, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("update_kcp"), &PacketPeerKCP::update_kcp);
	ClassDB::bind_method(D_METHOD("get_kcp_conv"), &PacketPeerKCP::get_kcp_conv);
	ClassDB::bind_method(D_METHOD("set_kcp_mode", "p_mode"), &PacketPeerKCP::set_kcp_mode);
	ClassDB::bind_method(D_METHOD("set_udp_peer", "p_udp_peer"), &PacketPeerKCP::set_udp_peer);
	ClassDB::bind_method(D_METHOD("get_udp_peer"), &PacketPeerKCP::get_udp_peer);
	ClassDB::bind_static_method("PacketPeerKCP", D_METHOD("create_from_udp", "p_udp_peer"), &PacketPeerKCP::create_from_udp);
}

Error PacketPeerKCP::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	int ret = ikcp_send(kcp, reinterpret_cast<const char *>(p_buffer), p_buffer_size);
	ERR_FAIL_COND_V(ret < 0, ERR_CONNECTION_ERROR);
	return OK;
}

Error PacketPeerKCP::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	ERR_FAIL_COND_V(udp_peer.is_null(), ERR_UNCONFIGURED);
	const char *buffer = nullptr;
	int size = 0;
	Error err = udp_peer->get_packet(reinterpret_cast<const uint8_t **>(&buffer), size);
	ERR_FAIL_COND_V(err != OK, err);
	ERR_FAIL_COND_V(ikcp_input(kcp, buffer, size) < 0, ERR_CONNECTION_ERROR);
	int recv_size = ikcp_peeksize(kcp);
	ERR_FAIL_COND_V(recv_size < 0, ERR_CONNECTION_ERROR);
	char *recv = (char *)memalloc(recv_size);
	recv_size = ikcp_recv(kcp, recv, recv_size);
	ERR_FAIL_COND_V(recv_size < 0, ERR_CONNECTION_ERROR);
	*r_buffer = reinterpret_cast<const uint8_t *>(recv);
	r_buffer_size = recv_size;
	return OK;
}

int PacketPeerKCP::get_available_packet_count() const {
	ERR_FAIL_COND_V(udp_peer.is_null(), 0);
	return udp_peer->get_available_packet_count();
}

int PacketPeerKCP::get_max_packet_size() const {
	ERR_FAIL_COND_V(udp_peer.is_null(), 0);
	return udp_peer->get_max_packet_size();
}

void PacketPeerKCP::init_kcp(int p_conv, int p_mode) {
	if (kcp) {
		ikcp_release(kcp);
		kcp = nullptr;
	}
	kcp = ikcp_create((uint32_t)p_conv, this);
	set_kcp_mode(p_mode);
	kcp->output = &kcp_output_callback;
}

void PacketPeerKCP::update_kcp() {
	ERR_FAIL_COND(!kcp);
	ikcp_update(kcp, Time::get_singleton()->get_ticks_msec());
}

int PacketPeerKCP::get_kcp_conv() const {
	ERR_FAIL_COND_V(!kcp, -1);
	return ikcp_getconv(kcp);
}

void PacketPeerKCP::set_kcp_mode(int p_mode) {
	ERR_FAIL_COND(!kcp);
	switch (p_mode) {
		case 0:
			ikcp_nodelay(kcp, 0, 40, 2, 1);
			break;
		case 1:
			ikcp_nodelay(kcp, 0, 30, 2, 1);
			break;
		case 2:
			ikcp_nodelay(kcp, 1, 20, 2, 1);
			break;
		case 3:
			ikcp_nodelay(kcp, 1, 10, 2, 1);
			break;
		default:
			ikcp_nodelay(kcp, 0, 40, 2, 1);
			break;
	}
}

void PacketPeerKCP::set_udp_peer(Ref<PacketPeerUDP> p_udp_peer) {
	udp_peer = p_udp_peer;
}

Ref<PacketPeerUDP> PacketPeerKCP::get_udp_peer() const {
	return udp_peer;
}

Ref<PacketPeerKCP> PacketPeerKCP::create_from_udp(Ref<PacketPeerUDP> p_udp_peer) {
	Ref<PacketPeerKCP> peer = memnew(PacketPeerKCP);
	peer->set_udp_peer(p_udp_peer);
	return peer;
}

PacketPeerKCP::~PacketPeerKCP() {
	if (kcp) {
		ikcp_flush(kcp);
		ikcp_release(kcp);
		kcp = nullptr;
	}
}
