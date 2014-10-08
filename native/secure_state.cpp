/*
 * secure_state.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: vova
 */

#include "secure_state.h"
#include "secure_socket.h"
#include "config.h"
#include "worker.h"

IdleState idleState;
AcceptingState accepting;
ConnectingState connecting;
EstablishedState established;

size_t IdleState::send(SecureSocket* ctx) {
	return 0;
}
size_t IdleState::recv(SecureSocket* ctx) {
	return 0;
}
void IdleState::connect(SecureSocket* ctx) {
	SSL_set_connect_state(ctx->impl->connection);
	ctx->change_state(&connecting);
}
void IdleState::accept(SecureSocket* ctx) {
    SSL_set_accept_state(ctx->impl->connection);
	ctx->change_state(&accepting);
}

size_t ConnectingState::try_connect(SecureSocket* ctx) {
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->impl->connect();
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
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->impl->accept();
	ctx->send_queue->workItem->sending(true);
	if (ret == Socket::DONE) {
		ctx->change_state(&established);
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
	return ctx->impl->send();
}
size_t EstablishedState::recv(SecureSocket* ctx) {
	return ctx->impl->recv();
}

