#!/usr/bin/python

import socket
import sys
import threading
import time
import ssl
import logging
import struct
from key import KeyBuilder, Secret



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
        Connection.init(self)
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
    def __init__(self, parent, socket, secret):
        Pipe.__init__(self, parent, socket)
        self.secret = secret
    def init(self):
        Pipe.init(self)
        try:
            self.s = ssl.wrap_socket(self.s,
                           server_side=True,
                           #certfile="cert.pem",
                           keyfile=self.secret,
                           ssl_version=ssl.PROTOCOL_SSLv23)
            log("Server connected")
        except:
            log("SSL handshake failed")
            KeyMngr.instance.forget(self.s.getpeername())
            self.s = self.s.unwrap()
            self.stop()
            self.s.shutdown()



class KeyServer(Connection):
    WAIT_HI = 0
    WAIT_BY = 1

    def __init__(self, parent, socket):
        Connection.__init__(self, parent, socket)
        self.state = KeyServer.WAIT_HI
        self.public_key = None
        self.key_builder = KeyBuilder(KeyMngr.name(self.s.getpeername()))
        self.cipher_file = ''
    def encode(self, key, data):
        return self.key_builder.encode(key, data)
    def create_cipher(self):
        cipher, self.cipher_file = self.key_builder.generate_cipher()
        #return Secret.HI + self.serialize_size(len(cipher)) + cipher
        return cipher
    def recv(self):
        data = self._recv()
        log("KeyServer received %s bytes"%len(data))
        if(self.state == KeyServer.WAIT_HI):
            self.public_key = self.parse_hello(data)
            log("Public key: %s"%self.public_key)
            if not self.public_key:
                log("Brocken HI, close connection")
                return False
            cipher = self.create_cipher()

            encoded = self.encode(self.public_key, cipher)
            log("Gonna send, bytes: %d"%len(encoded))

            self.s.send(encoded)

            log("Sent")
            self.state = KeyServer.WAIT_BY
            return True
        else:
            if self.is_ack_valid(data):
                KeyMngr.instance.remember(self.s.getpeername(), self.cipher_file)
                log("Key session done")
            else:
                log("Key session fail!")
            return False
    def parse_size(self, data):
        return ((ord(data[0])<<8) | (ord(data[1])))
    def parse_hello(self, data):
        hi = Secret.HI
        hi_len = len(hi)
        SIZE_FIELD = 2
        if len(data) >= len(hi) + SIZE_FIELD:
            if data.startswith(hi):
                size_field = data[hi_len:hi_len+SIZE_FIELD]
                public_key_length = self.parse_size(size_field)

                if len(data) == hi_len + SIZE_FIELD + public_key_length:
                    public_key = data[hi_len + SIZE_FIELD:]

                    return public_key
        return None
    def is_ack_valid(self, data):
        return data == Secret.BY


class KeyMngr:
    instance = None

    @staticmethod
    def create():
        KeyMngr.instance = KeyMngr()
    @staticmethod
    def name(addr):
        return addr[0]
    def __init__(self):
        self.map = {}
    def is_known(self, addr):
        logging.debug('Is known: %s {%s}'%(self.name(addr), self.map))
        return self.name(addr) in self.map
    def remember(self, addr, key_file):
        logging.debug('Remembder [%s]=%s'%(self.name(addr), key_file))
        self.map[self.name(addr)] = key_file
    def forget(self, addr):
        del self.map[self.name(addr)]
    def get(self, addr):
        self.map[self.name(addr)]



class Listener:
    def __init__(self, server_url, client_url):
        self.ip, self.port = server_url
        self.client_url = client_url
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

                if KeyMngr.instance.is_known(addr):
                    s = Server(self, socket, KeyMngr.instance.get(addr))
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


if len(sys.argv) >= 3:
    logging.basicConfig( level=logging.DEBUG,
                                format='%(asctime)s %(message)s',
                                filename='ser_sec.log',
                                filemode='a' )

    server = sys.argv[1]
    client = sys.argv[2]
    KeyMngr.create()

    ps = Listener(server.split(':'), client.split(':'))
    ps.start()
else:
    print("USAGE: serverip:port clientip:port")



