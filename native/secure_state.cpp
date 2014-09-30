/*
 * secure_state.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: vova
 */

#include "secure_state.h"
#include "secure_socket.h"
#include "config.h"

IdleState idleState;
AcceptingState accepting;
ConnectingState connecting;
EstablishedState established;

bool IdleState::is_sending(SecureSocket* ctx) {
	return false;
}
size_t IdleState::send(SecureSocket* ctx, char* buf, size_t size) {
	return 0;
}
size_t IdleState::recv(SecureSocket* ctx, char* buf, const size_t size) {
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

bool ConnectingState::is_sending(SecureSocket* ctx) {
	return true;
}
size_t ConnectingState::try_connect(SecureSocket* ctx, char* buf, size_t size) {
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->impl->connect();
	LOG.debugStream() << "SSL_connect=" << ret;
	if (ret == Socket::DONE) {
		ctx->change_state(&established);
	}
	return ret;
}
size_t ConnectingState::send(SecureSocket* ctx, char* buf, size_t size) {
	return try_connect(ctx, buf, size);
}
size_t ConnectingState::recv(SecureSocket* ctx, char* buf, const size_t size) {
	return try_connect(ctx, buf, size);
}

bool AcceptingState::is_sending(SecureSocket* ctx) {
	return true;
}
size_t AcceptingState::try_accept(SecureSocket* ctx, char* buf, size_t size) {
	Socket::SocketReturns ret = (Socket::SocketReturns)ctx->impl->accept();
	if (ret == Socket::DONE) {
		ctx->change_state(&established);
	}
	return ret;
}
size_t AcceptingState::send(SecureSocket* ctx, char* buf, size_t size) {
	return try_accept(ctx, buf, size);
}
size_t AcceptingState::recv(SecureSocket* ctx, char* buf, const size_t size) {
	return try_accept(ctx, buf, size);
}

bool EstablishedState::is_sending(SecureSocket* ctx) {
	return false;
}
size_t EstablishedState::send(SecureSocket* ctx, char* buf, size_t size) {
	return ctx->impl->send(buf, size);
}
size_t EstablishedState::recv(SecureSocket* ctx, char* buf, const size_t size) {
	return ctx->impl->recv(buf, size);
}

