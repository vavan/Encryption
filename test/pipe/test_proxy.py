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

BUFFER_SIZE = 4096


def TOTEXT(x):
    return chr(x%94+32)

class Thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.running = False
    def stop(self):
        self.running = False
    def done(self):
        pass
    def run(self):
        self.running = True
        while self.running:
            self.do()
        self.done()

class EchoServer(Thread):
    def __init__(self, socket, parent):
        Thread.__init__(self)
        self.s = socket
        self.parent = parent
    def do(self):
        data = self.s.recv(BUFFER_SIZE)
        self.parent.recvd += len(data)
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
        self.recvd = 0
    def listen(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((self.addr, int(self.port)))
        self.s.listen(100)
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
    def __init__(self, ip = CONNECT_TO_ADDR, port = CONNECT_TO_PORT):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((ip, int(port)))
    def send(self, data):
        self.s.send(data)
    def recv(self):
        try:
            return self.s.recv(BUFFER_SIZE)
        except Exception, e:
            print "Receive failed:", str(e)
        return ''
    def close(self):
        self.s.close()

class ClientThread(Thread):
    def __init__(self, request):
        Thread.__init__(self)
        self.request = request
        self.response = []
        self.index = 0
    def run(self):
        self.running = True
        c = Client()
        while self.running and self.index < len(self.request):
            c.send(self.request[self.index])
            self.response.append(c.recv())
            self.index += 1
        self.stop()
        c.close()
        

class BasicTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.listener = Listener()
        cls.listener.start()

    @classmethod
    def tearDownClass(cls):
        cls.listener.stop()

    def test_simple_ok(self):
        c = Client()
        request = 'ok';
        c.send(request)
        response = c.recv()
        c.close()
        self.assertEqual(request, response)


class MainTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.listener = Listener()
        cls.listener.start()

    @classmethod
    def tearDownClass(cls):
        cls.listener.stop()

    #@unittest.skip("Not now")
    def test_same_connect_128(self):
        cycles = 128
        a_request = 'Z'*128
        request = []
        response = []
        c = Client()
        for i in range(cycles):
            request.append(a_request)
            c.send(a_request)
            response.append(c.recv())
        c.close()
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_diff_connect_128(self):
        cycles = 128
        a_request = 'Z'*128
        request = []
        response = []
        for i in range(cycles):
            c = Client()
            request.append(a_request)
            c.send(a_request)
            response.append(c.recv())
            c.close()
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_same_connect_4096to1(self):
        request = []
        response = []
        c = Client()
        for i in range(4096, 1, 10):
            a_request = TOTEXT(i)*i
            request.append(a_request)
            c.send(a_request)
            response.append(c.recv())
        c.close()          
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_same_connect_1to4096(self):
        request = []
        response = []
        c = Client()
        for i in range(1, 4096, 10):
            a_request = TOTEXT(i)*i
            request.append(a_request)
            c.send(a_request)
            response.append(c.recv())
        c.close()          
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_thread_same_2x4(self):
        cycles = 2
        a_request = ['COOL', ]
        request = []
        response = []
        threads = []
        for i in range(cycles):
            request.append(a_request)
            c = ClientThread(a_request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response)            
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_thread_same_128x128(self):
        cycles = 128
        a_request = ['Q'*128, ]
        request = []
        response = []
        threads = []
        for i in range(cycles):
            request.append(a_request)
            c = ClientThread(a_request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response)             
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_thread_sequance_10x20x128x128(self):
        outer_cycles = 10
        cycles = 20
        request = []
        response = []
        for outer in range(outer_cycles):
            threads = []
            for i in range(cycles):
                a_request = [ TOTEXT(i)*128, ]*128
                request.append(a_request)
                c = ClientThread(a_request)
                c.start()
                threads.append(c)
            for t in threads:
                t.join()
                response.append(t.response)
        self.assertEqual(request, response)         

class LongRunningTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.listener = Listener()
        cls.listener.start()

    @classmethod
    def tearDownClass(cls):
        cls.listener.stop()


    #@unittest.skip("Not now")
    def test_repeat_long_seq(self):
        cycles = 100000
        c = Client()
        request = 'Z'*256
        response = []
        for i in range(cycles):
            c.send(request)
            recvd = c.recv()
            response.append((recvd == request))
        c.close()
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertTrue(response[i])



if __name__ == '__main__':
    if len(sys.argv) > 1:
        type = sys.argv[1]
    else:
        type = "main"
    if (type == "basic"):
        suite = unittest.TestLoader().loadTestsFromTestCase(BasicTests)
    elif (type == "profile"):
        suite = unittest.TestLoader().loadTestsFromTestCase(LongRunningTests)
    else:
        suite = unittest.TestLoader().loadTestsFromTestCase(MainTests)
    unittest.TextTestRunner(verbosity=2).run(suite)



