#!/usr/bin/python

import socket
import sys
import threading, time
import ssl
import logging

logging.basicConfig(level=logging.DEBUG,
                        format='%(asctime)s %(message)s',
                        filename='sec.log',
                        filemode='a'
                        )


def log(msg):
    logging.error(msg)

class RecordMixIn:
    fn = None
    def __init__(self):
        self.unsecure = None
        self.fn = RecordMixIn.fn
        if self.fn:
            self.f = open(self.fn, 'wb+')
    def __del__(self):
        if self.fn:
            self.f.close()
    def wsend(self, data):
        if self.fn:
            self.f.write("\r\n-->>--\r\n")
            self.f.write(data)
    def wrecv(self, data):
        if self.fn:
            self.f.write("\r\n--<<--\r\n")
            self.f.write(data)



class Pipe(threading.Thread):
    BUFFER_SIZE = 10000
    def __init__(self, parent, secure, socket = None):
        threading.Thread.__init__(self)
        self.parent = parent
        self.secure = secure
        self.s = socket
        self.queue = []
        self.running = False
    def __del__(self):
        if self.s != None:
            self.s.close()
    def join_pipe(self, other):
        self.other = other
        other.other = self
    def recv(self):
        data = self.s.recv(Pipe.BUFFER_SIZE)
        log("Pipe received %s bytes"%len(data))
        if data:
            self.other.send(data)
        return data
    def unqueue(self):
        for i in self.queue:
            log("Pipe un-queued %d bytes"%len(i))
            self.s.send(i)
        self.queue = []
    def send(self, data):
        if self.running == False:
            log("Pipe queued %d bytes"%len(data))
            self.queue.append(data)
        else:
            log("Pipe send %d bytes"%len(data))
            self.s.send(data)
    def stop(self):
        self.running = False
    def create(self):
        pass
    def run(self):
        self.create()
        self.running = True
        self.unqueue()
        while self.running:
            try:
                if not self.recv():
                    log("Pipe disconnected")
                    break
            except socket.timeout as e:
                log("***: %s"%str(e))
                break
            except socket.error as e:
                log("***: %s"%str(e))
                break
        log("Pipe closed")
        self.stop()
        self.other.stop()
        self.s.close()
        self.s = None
        self.parent.disconnect(self)

class Client(Pipe):
    def __init__(self, parent, secure, ip, port):
        Pipe.__init__(self, parent, secure)
        self.ip, self.port = (ip, port)
    def create(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.secure:
            self.s = ssl.wrap_socket(self.s,
                               ca_certs="cert.pem",
                               cert_reqs=ssl.CERT_REQUIRED)
        self.s.connect((self.ip, int(self.port)))
        log("Client connected")

class Server(Pipe):
    def __init__(self, parent, secure, socket):
        Pipe.__init__(self, parent, secure, socket)
    def create(self):
        if self.secure:
            self.s = ssl.wrap_socket(self.s,
                               server_side=True,
                               certfile="cert.pem",
                               keyfile="cert.pem",
                               ssl_version=ssl.PROTOCOL_SSLv23)
        log("Server connected")

class DumpServer(Server, RecordMixIn):
    def __init__(self, parent, secure, socket):
        Server.__init__(self, parent, secure, socket)
        RecordMixIn.__init__(self)
        self.unsecure = self.s
    def recv(self):
        data = self.unsecure.recv(Pipe.BUFFER_SIZE)
        log("Unsecured Pipe received %s bytes"%len(data))
##        if data:
##            self.other.send(data)
        return data

class KeyMngr:
    pass

class Listener:
    def __init__(self, server_url, client_url, Server_Class):
        self.secure, self.ip, self.port = server_url
        self.client_url = client_url
        self.Server_Class = Server_Class
        self.children = []
    def create(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((self.ip, int(self.port)))
        s.listen(1)
        return s
    def stop(self):
        for i in self.children:
            i.stop()
    def disconnect(self, child):
        self.children.remove( child )
    def start(self):
        listen = self.create()
        while 1:
            try:
                socket, addr = listen.accept()
                log("Accepted connection from %s"%str(addr))

                c = Client(self, *self.client_url)
                s = self.Server_Class(self, self.secure, socket)

                self.children.append( c )
                self.children.append( s )
                c.join_pipe(s)
                c.start()
                s.start()
            except KeyboardInterrupt:
                break
        listen.close()
        self.stop()
        print("EXIT")
        time.sleep(1)


def parse_url(url):
    if url.startswith('s'):
        return [True,] + url[1:].split(':')
    else:
        return [False,] + url.split(':')

if len(sys.argv) >= 3:
    server = sys.argv[1]
    client = sys.argv[2]
    if len(sys.argv) == 4:
        dump = sys.argv[3]
        Server_Class = DumpServer
        RecordMixIn.fn = dump
    else:
        Server_Class = Server

    ps = Listener(parse_url(server), parse_url(client), Server_Class)
    ps.start()
else:
    print("USAGE: [s]serverip:port [s]clientip:port [dump]")



