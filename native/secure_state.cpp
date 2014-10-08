/*
 * secure_state.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: vova
 */

#include "secure_state.h"
#include "secure_socket.h"
#include "queue.h"
#include "worker.h"
#include "config.h"

AcceptingState accepting;
ConnectingState connecting;
EstablishedState established;


size_t ConnectingState::try_connect(SecureSocket* ctx) {
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->do_connect();
	ctx->send_queue->workItem->sending(true);
	if (ret == Socket::DONE) {
		ctx->change_state(&established);
		ctx->send_queue->workItem->sending(false);
	}
	return ret;
}
size_t ConnectingState::send(SecureSocket* ctx) {
	return try_connect(ctx);
}
size_t ConnectingState::recv(SecureSocket* ctx) {
	return try_connect(ctx);
}


size_t AcceptingState::try_accept(SecureSocket* ctx) {
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->do_accept();
	ctx->send_queue->workItem->sending(true);
	if (ret == Socket::DONE) {
		ctx->change_state(&established);
		//TODO Refactor this weirdness
		ret = Socket::INPROGRESS;
		ctx->send_queue->workItem->sending(false);
	}
	return ret;
}
size_t AcceptingState::send(SecureSocket* ctx) {
	return try_accept(ctx);
}
size_t AcceptingState::recv(SecureSocket* ctx) {
	return try_accept(ctx);
}


size_t EstablishedState::send(SecureSocket* ctx) {
	return ctx->do_send();
}
size_t EstablishedState::recv(SecureSocket* ctx) {
	return ctx->do_recv();
}

