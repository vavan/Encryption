#!/usr/bin/python

import sys, os
import subprocess, socket
import time
import unittest
import threading

LOCALHOST = '127.0.0.1'
LISTEN_ON_ADDR = LOCALHOST
LISTEN_ON_PORT = 6000
CONNECT_TO_ADDR = LOCALHOST
CONNECT_TO_PORT = 2000


class Thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.running = False
    def stop(self):
        self.running = False
    def run(self):
        self.running = True
        while self.running:
            self.do()
        self.done()

class EchoServer(Thread):
    BUFFER_SIZE = 2048
    def __init__(self, socket, parent):
        Thread.__init__(self)
        self.s = socket
        self.parent = parent
    def do(self):
        data = self.s.recv(EchoServer.BUFFER_SIZE)
        if data:
            self.s.send(data)
        else:
            self.stop()
            self.parent.disconnect(self)
    def done(self):
        self.s.close()    

class Listener(Thread):
    def __init__(self, addr = LISTEN_ON_ADDR, port = LISTEN_ON_PORT):
        Thread.__init__(self)
        self.addr = addr
        self.port = port
        self.children = []
        self.running = False
        self.listen()
    def listen(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((self.addr, int(self.port)))
        self.s.listen(0)
        self.s.settimeout(1)
    def stop(self):
        Thread.stop(self)
        for i in self.children:
            i.stop()
    def disconnect(self, child):
        self.children.remove( child )
    def do(self):
        try:
            s, addr = self.s.accept()
            echo = EchoServer(s, self)
            self.children.append( echo )
            echo.start()
        except socket.timeout:
            pass
    def done(self):
        self.s.close()


class Client:
    BUFFER_SIZE = 2048
    def __init__(self, ip = CONNECT_TO_ADDR, port = CONNECT_TO_PORT):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((ip, int(port)))
    def send(self, data):
        self.s.send(data)
    def recv(self):
        return self.s.recv(Client.BUFFER_SIZE)
    def close(self):
        self.s.close()



class ProxyTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.listener = Listener()
        cls.listener.start()

    @classmethod
    def tearDownClass(cls):
        cls.listener.stop()

    #@unittest.skip("Not now")
    def test_simple_ok(self):
        c = Client()
        request = 'ok';
        c.send(request)
        response = c.recv()
        c.close()
        self.assertEqual(request, response)


    @unittest.skip("Not now")
    def test_simple_128(self):
        c = Client()
        request = 'Z'*128;
        c.send(request)
        response = c.recv()
        c.close()
        self.assertEqual(request, response)

    @unittest.skip("Not now")
    def test_repeat_128(self):
        cycles = 128
        c = Client()
        request = 'Z'*128;
        response = []
        for i in range(cycles):
            c.send(request)
            response.append(c.recv())
        c.close()
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(ProxyTests)
    unittest.TextTestRunner(verbosity=2).run(suite)
    #unittest.main()



