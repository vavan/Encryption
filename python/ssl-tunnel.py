#!/usr/bin/python

import socket
import sys
import threading, time
import ssl
import logging


SOCKET_TIMEOUT = 3

logging.basicConfig(level=logging.DEBUG,
                        format='%(asctime)s %(message)s',
                        filename='ssl-tunel.log',
                        filemode='a'
                        )


def log(msg):
    logging.error(msg)


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
        #log("ZZZZPipe received %s bytes. %s"%(len(data), self.s))
        #log("[[%s]]"%data)
        if data:
            #log("QQQQPipe send %s bytes. %s"%(len(data), self.s))
            self.other.send(data)
        return data
    def unqueue(self):
        for i in self.queue:
            #log("Pipe un-queued %d bytes"%len(i))
            self.s.send(i)
        self.queue = []
    def send(self, data):
        if self.running == False:
            #log("Pipe queued %d bytes"%len(data))
            self.queue.append(data)
        else:
            #log("Pipe send %d bytes"%len(data))
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
                    log("Pipe disconnected, %s"%self.s)
                    break
            except socket.timeout as e:
                log("***: %s"%str(e))
                pass
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
        self.s.settimeout(SOCKET_TIMEOUT)
        log("Client connected")


class Server(Pipe):
    def __init__(self, parent, secure, socket):
        Pipe.__init__(self, parent, secure, socket)
    def create(self):
        self.s.settimeout(SOCKET_TIMEOUT)
        if self.secure:
            self.s = ssl.wrap_socket(self.s,
                               server_side=True,
                               certfile="cert.pem",
                               keyfile="key.pem",
                               ssl_version=ssl.PROTOCOL_SSLv23)
        log("Server connected")



class Listener:
    def __init__(self, server_url, client_url):
        self.secure, self.ip, self.port = server_url
        self.client_url = client_url
        self.children = []
    def create(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((self.ip, int(self.port)))
        s.listen(100)
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

                s = Server(self, self.secure, socket)
                self.children.append( s )

                c = Client(self, *self.client_url)
                self.children.append( c )
                c.join_pipe(s)
                s.start()
                c.start()
            except KeyboardInterrupt:
                break
        self.stop()
        listen.close()
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

    ps = Listener(parse_url(server), parse_url(client))
    ps.start()
else:
    print("USAGE: [s]serverip:port [s]clientip:port")



