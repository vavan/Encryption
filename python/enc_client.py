#!/usr/bin/python

import sys
import logging
import socket
from key import KeyBuilder, Secret



class KeyClient:
    BUFFER_SIZE = 16384
    WAIT_HI = 0
    WAIT_BY = 1

    def __init__(self, ip, port):
        self.ip, self.port = (ip, port)
        self.key_builder = KeyBuilder('temp')
        self.key_builder.generate_private()
    def init(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.ip, int(self.port)))
        logging.debug("Client connected")
    def run(self):
        status = 0
        self.init()

        data = self.create_hi()
        self.s.send(data)

        data = self.s.recv(KeyClient.BUFFER_SIZE)
        if not data:
            logging.error('Dropped')
            status = 1
            return status
        cipher = self.parse_cipher(data)
        if cipher:
            self.save(cipher)
            self.s.send(Secret.BY)
        else:
            logging.error('No secret!')
            status = 2

        self.s.close()
        return status
    def serialize_size(self, size):
        return chr(size>>8) + chr(size&0xFF)
    def create_hi(self):
        data = Secret.HI
        public = self.key_builder.generate_public()
        data += self.serialize_size(len(public))
        data += public
        return data
    def parse_cipher(self, data):
        return self.key_builder.decode(data)
    def save(self, cipher):
        f = open('key.pem', 'wb')
        f.write(cipher)
        f.close()


if len(sys.argv) >= 2:
    logging.basicConfig(level=logging.DEBUG,
                            format='%(asctime)s %(message)s',
                            filename='cli_sec.log',
                            filemode='a')

    server = sys.argv[1]
    exit(KeyClient(*server.split(':')).run())
else:
    print("USAGE: serverip:port")


