#!/usr/bin/python

import socket
import sys
import threading
import time
import ssl
import logging
import struct
#from key import KeyBuilder, Secret

from subprocess import Popen, PIPE, STDOUT


class Secret:
    HI = "Slava Ukraini!"
    BY = "Geroyam Slava!"


class KeyBuilder:
    GENERATE_KEY = 'openssl genrsa -out %(secret)s.pem 512'
    GENERATE_CERT = 'openssl x509 -req -days 365 -in server.csr -signkey %(secret)s.pem -out %(secret)s.crt'
    GENERATE_PRIVATE = 'openssl genrsa 4096'
    GENERATE_PUBLIC = 'openssl rsa -outform PEM -pubout'
    ENCODE = 'openssl rsautl -encrypt -inkey %(public)s -pubin'
    DECODE = 'openssl rsautl -decrypt -inkey %(private)s'

    def __init__(self, name):
        self.name = name
        self.private = None
    def __execute(self, cmd, indata = ''):
        logging.debug("Execute: %s"%cmd)
        p = Popen(cmd, shell = True, stdin=PIPE, stdout=PIPE, stderr = STDOUT)
        if indata:
            p.stdin.write(indata)
            p.stdin.close()
        return p.stdout.read()
    def generate_cipher(self):
        cmd = KeyBuilder.GENERATE_KEY%{'secret': self.name}
        self.__execute(cmd)
        cmd = KeyBuilder.GENERATE_CERT%{'secret': self.name}
        self.__execute(cmd)
        cipher = open(self.name+'.pem', 'rb').read()
        return (cipher, self.name)
    def generate_private(self):
        cmd = KeyBuilder.GENERATE_PRIVATE
        self.private = self.__execute(cmd)
        logging.debug("Private: %s"%self.private)
    def generate_public(self):
        cmd = KeyBuilder.GENERATE_PUBLIC
        return self.__execute(cmd, self.private)
    def encode(self, key = None, data = None):
        logging.debug("Encoding")
        keyfile = self.name+'_pub.pem'
        f = open(keyfile, 'wb')
        f.write(key)
        f.close()
        cmd = KeyBuilder.ENCODE%{'public': keyfile}
        encoded = self.__execute(cmd, data)
        #logging.debug("Encoded: %s %s"%(data, encoded))

        #os.unlink(keyfile)
        return encoded
    def decode(self, data):
        keyfile = self.name+'_pri.pem'
        f = open(keyfile, 'wb')

        f.write(self.private)
        f.close()
        cmd = KeyBuilder.DECODE%{'private': keyfile}
        decoded = self.__execute(cmd, data)
        #os.unlink(keyfile)
        return decoded


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
        logging.debug("Start %s"%self)
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
        log("Closed %s"%self)
        self.parent.disconnect(self)
        if self.s != None:
            self.s.close()
            self.s = None


class Pipe(Connection):
    def __init__(self, parent, socket = None):
        Connection.__init__(self, parent, socket)
        self.queue = []
        self.other = None
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
            if self.s:
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
        Pipe.init(self)
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.ip, int(self.port)))
        log("Client connected")

class Server(Pipe):
    def __init__(self, parent, socket, secret):
        Pipe.__init__(self, parent, socket)
        self.secret = secret
    def init(self):
        Pipe.init(self)
        cert = self.secret + '.crt'
        key = self.secret + '.pem'
        logging.debug('wrap socket key: %s %s'%(cert, key))
        self.s = ssl.wrap_socket(self.s,
                       server_side=True,
                       certfile=cert,
                       keyfile=key,
                       do_handshake_on_connect=False,
                       ssl_version=ssl.PROTOCOL_SSLv23)
        try:
            self.s.do_handshake()
            log("Server connected")
        except ssl.SSLError, e:
            log("SSL handshake failed: %s"%str(e))
            #self.s = self.s.unwrap()
            self.stop()
            self.s.shutdown(socket.SHUT_RDWR)



class KeyServer(Connection):
    WAIT_HI = 0
    WAIT_BY = 1

    def __init__(self, parent, socket):
        Connection.__init__(self, parent, socket)
        self.state = KeyServer.WAIT_HI
        self.public_key = None
        self.key_builder = KeyBuilder(self.build_name())
        self.cipher_file = ''
    def build_name(self):
        name = self.s.getpeername()
        name = name[0]+'_'+str(name[1])
        return name
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
                log("Key session done")
                self.switch_to_proxy()
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
                    log("Hello parsed. Total=%s, key size=%s"%(len(data), public_key_length))  

                    return public_key
        return None
    def is_ack_valid(self, data):
        return data == Secret.BY
    def release_socket(self):
        s = self.s
        self.s = None
        return s
    def switch_to_proxy(self):
        s = Server(self.parent, self.release_socket(), self.cipher_file)
        self.parent.children.append( s )
        s.start()

        c = Client(self.parent, *self.parent.client_url)
        self.parent.children.append( c )
        c.join_pipe(s)
        c.start()
        self.stop()


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

    ps = Listener(server.split(':'), client.split(':'))
    ps.start()
else:
    print("USAGE: serverip:port clientip:port")



