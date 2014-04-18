#!/usr/bin/python

import socket
import sys
import threading, time
import ssl

debug = 0
def log(msg):
    if debug:
        print msg

class Base:
    BUFFER_SIZE = 10000
    def __init__(self, secure = None, ip = None, port = 0):
        self.secure = secure
        self.ip = ip
        self.port = int(port)        
        self.running = False
        self.debug = False
    def set_pipe(self, other):
        self.other = other
        other.other = self
    def recv(self, data):
        self.other.send(data)

class Record(Base):
    def __init__(self, fn):
        Base.__init__(self)
        self.fn = fn
        self.f = open(fn, 'wb+')
    def __del__(self):
        self.f.close()
    def send(self, data):
        self.f.write(data)
    def start(self):
        pass

class Client(Base, threading.Thread):
    def __init__(self, secure, ip, port):
        Base.__init__(self, secure, ip, port)
        threading.Thread.__init__(self)
        self.queue = []
        self.s = None
    def __del__(self):
        if self.s != None:
            self.s.close()
    def unqueue(self):
        for i in self.queue:
            log("Client un-queued %d bytes"%len(i))
            self.s.send(i)
        self.queue = []   
    def send(self, data):   
        if self.running == False:
            log("Client queued %d bytes"%len(data))       
            self.queue.append(data)
        else:
            log("Client send %d bytes"%len(data))
            self.s.send(data)
    def run(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.secure:
            self.s = ssl.wrap_socket(s,
                               ca_certs="cert.pem",
                               cert_reqs=ssl.CERT_REQUIRED)
        else:
            self.s = s
        self.s.connect((self.ip, self.port))
        log("Client connected")
        self.running = True
        self.unqueue()
        while self.running:
            try:
                data = self.s.recv(Base.BUFFER_SIZE)
                log("Client received %s bytes"%len(data))
                if not data:
                    break
                self.recv(data)
            except socket.timeout as e:
                print("***Client:", e)
                break
            except socket.error as e:
                print("***Client:", e)
                break
            except KeyboardInterrupt:
                self.other.running = False
                self.running = False
                print "CEXIT"
                break


class Server(Base):
    def __init__(self, secure, ip, port, client):
        Base.__init__(self, secure, ip, port)
        self.client = client
    def send(self, data):
        log("Server send %d bytes"%len(data))
        self.conn.send(data)
    def start(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((self.ip, self.port))
        self.s.listen(1)
        self.running = True
        while self.running:
            try:
                conn, addr = self.s.accept()
                if self.secure:
                    self.conn = ssl.wrap_socket(conn,
                                           server_side=True,
                                           certfile="cert.pem",
                                           keyfile="cert.pem",
                                           ssl_version=ssl.PROTOCOL_SSLv23)
                else:
                    self.conn = conn
                
            except KeyboardInterrupt:
                self.s.close()
                self.running = False
                self.client.running = False
                print "SEXIT"
                break
            self.client.start()
            self.client.set_pipe(self)
            log("Server accepted connection form %s"%str(addr))
            while 1:
                try:
                    data = self.conn.recv(Base.BUFFER_SIZE)
                    log("Server received: %d bytes"%len(data))
                    if not data:
                        self.client.running = False
                        break
                    self.recv(data)
                except socket.timeout as e:
                    print("Server", e)
                    break
                except socket.error as e:
                    print("Server", e)
                    break
                except KeyboardInterrupt:
                    self.s.close()
                    self.other.running = False
                    self.running = False
                    print "SEXIT"
                    break
            self.conn.close()
            time.sleep(1)


def parse_url(url):
    if url.startswith('s'):
        return [True,] + url[1:].split(':')
    else:
        return [False,] + url.split(':')

if len(sys.argv) == 3:
    server = sys.argv[1]
    client = sys.argv[2]
    if client.find(':') != -1:
        c = Client(*parse_url(client))
    else:
        c = Record(client)

    ps = Server(*parse_url(server), client=c)
    ps.start()
else:
    print "USAGE: Sip Sport Cip Cport"
    print "OR USAGE: Sip Sport store_to_file"


