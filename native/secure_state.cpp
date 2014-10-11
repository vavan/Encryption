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


Socket::RetCode ConnectingState::try_connect(SecureSocket* ctx) {
	Socket::RetCode ret = ctx->do_connect();
	if (ret == Socket::OK) {
		ctx->change_state(&established);
		ctx->send_queue->workItem->sending(false);
	} else {
		ctx->send_queue->workItem->sending(true);
	}
	return ret;
}
Socket::RetCode ConnectingState::send(SecureSocket* ctx) {
	return try_connect(ctx);
}
Socket::RetCode ConnectingState::recv(SecureSocket* ctx) {
	return try_connect(ctx);
}


Socket::RetCode AcceptingState::try_accept(SecureSocket* ctx) {
	Socket::RetCode ret = ctx->do_accept();
	if (ret == Socket::OK) {
		ctx->change_state(&established);
		ctx->send_queue->workItem->sending(false);
	} else {
		ctx->send_queue->workItem->sending(true);
	}
	return ret;
}
Socket::RetCode AcceptingState::send(SecureSocket* ctx) {
	return try_accept(ctx);
}
Socket::RetCode AcceptingState::recv(SecureSocket* ctx) {
	return try_accept(ctx);
}


Socket::RetCode EstablishedState::send(SecureSocket* ctx) {
	return ctx->do_send();
}
Socket::RetCode EstablishedState::recv(SecureSocket* ctx) {
	return ctx->do_recv();
}

