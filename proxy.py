#!/usr/bin/python

import socket
import sys
import threading
import time
import ssl
import logging
import struct

logging.basicConfig(level=logging.DEBUG,
                        format='%(asctime)s %(message)s',
                        filename='sec.log',
                        filemode='a'
                        )


def log(msg):
    logging.error(msg)

class Connection(threading.Thread):
    BUFFER_SIZE = 16384
    def __init__(self, parent, socket = None):
        threading.Thread.__init__(self)
        self.parent = parent
        self.s = socket
        self.running = False
    def __del__(self):
        if self.s != None:
            self.s.close()
    def _recv(self):
        return self.s.recv(Pipe.BUFFER_SIZE)
    def stop(self):
        self.running = False
    def init(self):
        self.running = True
    def run(self):
        self.init()
        while self.running:
            try:
                if not self.recv():
                    log("Disconnected")
                    break
            except socket.timeout as e:
                log("***: %s"%str(e))
                break
            except socket.error as e:
                log("***: %s"%str(e))
                break
        log("Closed")
        self.stop()
        self.s.close()
        self.s = None
        self.parent.disconnect(self)


class Pipe(Connection):

    def __init__(self, parent, socket = None):
        Connection.__init__(self, parent, socket)
        self.queue = []
    def join_pipe(self, other):
        self.other = other
        other.other = self
    def recv(self):
        data = self._recv()
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
    def init(self):
        super.init(self)
        self.unqueue()

class Client(Pipe):
    def __init__(self, parent, ip, port):
        Pipe.__init__(self, parent)
        self.ip, self.port = (ip, port)
    def init(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.ip, int(self.port)))
        log("Client connected")

class Server(Pipe):
    def __init__(self, parent, socket):
        Pipe.__init__(self, parent, socket)
    def init(self):
        super.init(self)
        self.s = ssl.wrap_socket(self.s,
                           server_side=True,
                           certfile="cert.pem",
                           keyfile="cert.pem",
                           ssl_version=ssl.PROTOCOL_SSLv23)
        log("Server connected")


class KeyServer(Connection):
    INIT = 0
    PUBLIC = 1
    CIPHER = 2
    HI = "Slava Ukraini!"
    BY = "Heroyam Slava!"

    def __init__(self, parent, socket):
        super.__init__(self, parent, socket)
        self.state = KeyServer.INIT
        self.public_key = None
        self.hello = ''
    def recv(self):
        data = self._recv()
        log("KeyServer received %s bytes"%len(data))

        if(self.state == KeyServer.INIT):
            self.public_key = self.parse_hello(data)
            if self.public_key:
                self.state = KeyServer.PUBLIC
            return True
        elif (self.state == KeyServer.PUBLIC):
            cipher = self.create_cipher()
            encoded = self.encode(cipher, self.public_key)
            self.s.send(encoded)
            self.state = KeyServer.CIPHER
            return True
        else:
            if self.is_ack_valid(data):
                log("Key session done")
            else:
                log("Key session fail!")
            return False
    def parse_hello(self, data):
        self.hello += data
        if len(data) > len(self.HI + 2):
            if self.hello.startswith(self.HI):
                public_key_length = struct.unpack('H', data[len(self.HI):len(self.HI)+2])
                if len(data) > len(self.HI) + 2 + public_key_length:
                    public_key = data[len(self.HI) + 2:]
                    return public_key
        return None
    def encode(self, data, key):
        pass
    def is_ack_valid(self, data):
        return data == self.BY



class KeyMngr:
    def __init__(self):
        pass
    def is_known(self, addr):
        pass


class Listener:
    def __init__(self, server_url, client_url, key_mngr):
        self.ip, self.port = server_url
        self.client_url = client_url
        self.children = []
        self.key_mngr = key_mngr
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

                if self.key_mngr.is_known(addr):
                    s = Server(self, socket)
                    self.children.append( s )
                    s.start()

                    c = Client(self, *self.client_url)
                    self.children.append( c )
                    c.join_pipe(s)
                    c.start()
                else:
                    s = KeyServer(self, socket)
                    self.children.append( s )
                    s.start()
            except KeyboardInterrupt:
                break
        self.stop()
        listen.close()
        print("EXIT")
        time.sleep(1)


def parse_url(url):
    return url.split(':')

if len(sys.argv) >= 3:
    server = sys.argv[1]
    client = sys.argv[2]
    key_mngr = KeyMngr()

    ps = Listener(parse_url(server), parse_url(client), key_mngr)
    ps.start()
else:
    print("USAGE: serverip:port clientip:port")



