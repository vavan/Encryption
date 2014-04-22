#!/usr/bin/python

import socket
import sys
import threading, time
import ssl

debug = 0
def log(msg):
    if debug:
        print(msg)


class DummyMixIn:
    def __init__(self, fn):
        self.unsecure = None
    def wsend(self, data):
        pass
    def wrecv(self, data):
        pass

class RecordMixIn:
    def __init__(self, fn):
        self.unsecure = None
        self.fn = fn
        if self.fn:
            self.f = open(fn, 'wb+')
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



class Pipe(threading.Thread, DummyMixIn):
    BUFFER_SIZE = 10000
    def __init__(self, parent, secure, socket = None):
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
    def recv(self, data):
        self.wrecv(data)
        self.other.send(data)
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
            self.wsend(data)
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
                data = self.s.recv(Base.BUFFER_SIZE)
                log("Pipe received %s bytes"%len(data))
                if not data:
                    log("Pipe disconnected")
                    break
                self.recv(data)
            except socket.timeout as e:
                print("***:", e)
                break
            except socket.error as e:
                print("***:", e)
                break
            except KeyboardInterrupt:
                self.other.running = False
                self.running = False
                print("EXIT")
                break
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
            self.s = ssl.wrap_socket(self.unsecure,
                               ca_certs="cert.pem",
                               cert_reqs=ssl.CERT_REQUIRED)
        self.s.connect((self.ip, self.port))
        log("Client connected")

class Server(Pipe, RecordMixIn):
    def __init__(self, parent, secure, socket, fn):
        Pipe.__init__(self, parent, secure, socket)
        RecordMixIn.__init__(self, fn)
    def create(self):
        if self.secure:
            self.unsecure = self.s
            self.s = ssl.wrap_socket(self.unsecure,
                               server_side=True,
                               certfile="cert.pem",
                               keyfile="cert.pem",
                               ssl_version=ssl.PROTOCOL_SSLv23)

class Listener:
    def __init__(self, server_url, client_url, dump = None):
        self.secure, self.ip, self.port = server_url
        self.client_url = client_url
        self.dump = dump
        self.children = []
    def create(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((self.ip, self.port))
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

                c = Client(self, *client_url)
                s = Server(self, self.secure, socket, self.dump)

                self.children.append( c )
                self.children.append( s )
                c.set_pipe(s)
                c.start()
                s.start()
            except KeyboardInterrupt:
                break
        listen.close()
        self.stop()
        print("EXIT")


def parse_url(url):
    if url.startswith('s'):
        return [True,] + url[1:].split(':')
    else:
        return [False,] + url.split(':')

if len(sys.argv) == 3:
    server = sys.argv[1]
    client = sys.argv[2]
    if len(sys.argv) == 4:
        dump = sys.argv[3]
    else:
        dump = None

    ps = Listener(parse_url(server), parse_url(client), dump)
    ps.start()
else:
    print("USAGE: [s]serverip:port [s]clientip:port [dump]")



