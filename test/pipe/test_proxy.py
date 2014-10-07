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
    def done(self):
        pass
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
    BUFFER_SIZE = 2048
    def __init__(self, ip = CONNECT_TO_ADDR, port = CONNECT_TO_PORT):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((ip, int(port)))
    def send(self, data):
        self.s.send(data)
    def recv(self):
        try:
            return self.s.recv(Client.BUFFER_SIZE)
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
        self.index = len(request) - 1
    def do(self):
        if (self.index >= 0):
            c = Client()
            c.send(self.request[self.index])
            self.response.append(c.recv())
            c.close()
            self.index -= 1
        else:
            self.stop()

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
    def test_simple_128(self):
        c = Client()
        request = 'Z'*128
        c.send(request)
        response = c.recv()
        c.close()
        self.assertEqual(request, response)

    #@unittest.skip("Not now")
    def test_repeat_same_128(self):
        cycles = 128
        c = Client()
        request = 'Z'*128
        response = []
        for i in range(cycles):
            c.send(request)
            response.append(c.recv())
        c.close()
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_repeat_diff_128(self):
        cycles = 128
        request = 'Z'*128
        response = []
        for i in range(cycles):
            c = Client()
            c.send(request)
            response.append(c.recv())
            c.close()
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_thread_same_2x4(self):
        cycles = 2
        request = ['COOL', ]
        response = []
        threads = []
        for i in range(cycles):
            c = ClientThread(request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response) 
            
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_thread_same_12x128(self):
        cycles = 12
        request = ['Q'*128, ]
        response = []
        threads = []
        for i in range(cycles):
            c = ClientThread(request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response) 
            
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_thread_same_128x12(self):
        cycles = 80
        request = ['Q'*128, ]
        response = []
        threads = []
        for i in range(cycles):
            c = ClientThread(request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response) 
            
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_thread_sequance(self):
        outer_cycles = 10
        cycles = 20
        request = ['Q'*32, ]
        response = []
        for outer in range(outer_cycles):
            threads = []
            for i in range(cycles):
                c = ClientThread(request)
                c.start()
                threads.append(c)
            for t in threads:
                t.join()
                response.append(t.response) 
            
        self.assertEqual(len(response), cycles*outer_cycles)
        for i in range(cycles*outer_cycles):
            self.assertEqual(request, response[i])

    #@unittest.skip("Not now")
    def test_thread_diff_128x12(self):
        cycles = 100
        request = []
        response = []
        threads = []
        for i in range(cycles):
            a_request = [ chr(i+0x40)*12, ]
            request.append(a_request)
            c = ClientThread(a_request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response) 
            
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request[i], response[i])

    #@unittest.skip("Not now")
    def test_thread_chr_30x30x30(self):
        cycles = 30
        request = []
        response = []
        threads = []
        for i in range(cycles):
            a_request = [ chr(i+0x40)*30, ]*30
            request.append(a_request)
            c = ClientThread(a_request)
            c.start()
            threads.append(c)
        for t in threads:
            t.join()
            response.append(t.response) 
            
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertEqual(request[i], response[i])


class LongRunningTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.listener = Listener()
        cls.listener.start()

    @classmethod
    def tearDownClass(cls):
        cls.listener.stop()
        #print "STOP:", cls.listener.recvd

    #@unittest.skip("Not now")
    def test_repeat_long_seq(self):
        cycles = 100000
        c = Client()
        request = 'Z'*2048
        response = []
        for i in range(cycles):
            c.send(request)
            #time.sleep(0.1)
            recvd = c.recv()
            response.append((recvd == request))
        c.close()
        self.assertEqual(len(response), cycles)
        for i in range(cycles):
            self.assertTrue(response[i])



if __name__ == '__main__':
    #suite = unittest.TestLoader().loadTestsFromTestCase(BasicTests)
    #suite = unittest.TestLoader().loadTestsFromTestCase(MainTests)
    suite = unittest.TestLoader().loadTestsFromTestCase(LongRunningTests)
    unittest.TextTestRunner(verbosity=2).run(suite)
    #unittest.main()



