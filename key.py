import os
import logging
from subprocess import Popen, PIPE


class Secret:
    HI = "Slava Ukraini!"
    BY = "Heroyam Slava!"


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
        p = Popen(cmd, shell = True, stdin=PIPE, stdout=PIPE)
        if indata:
            p.stdin.write(indata)
            p.stdin.close()
        return p.stdout.read()
    def generate_cipher(self):
        cmd = KeyBuilder.GENERATE_KEY%{'secret': self.name}
        self.__execute(cmd)
        cmd = KeyBuilder.GENERATE_CERT%{'secret': self.name}
        self.__execute(cmd)
        cipher = open(self.name+'.pem').read()
        return (cipher, self.name+'.pem')
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
        f = open(keyfile, 'w')
        f.write(key)
        f.close()
        cmd = KeyBuilder.ENCODE%{'public': keyfile}
        encoded = self.__execute(cmd, data)
        #os.unlink(keyfile)
        return encoded
    def decode(self, data):
        keyfile = self.name+'_pri.pem'
        f = open(keyfile, 'w')

        f.write(self.private)
        f.close()
        cmd = KeyBuilder.DECODE%{'private': keyfile}
        decoded = self.__execute(cmd, data)
        #os.unlink(keyfile)
        return decoded
