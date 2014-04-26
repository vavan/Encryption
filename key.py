import os
import logging
from subprocess import Popen, PIPE


class Secret:
    HI = "Slava Ukraini!"
    BY = "Heroyam Slava!"


class KeyBuilder:
    GENERATE_CERT = 'openssl genrsa -out %(secret)s 1024'
    GENERATE_PRIVATE = 'openssl genrsa 4096'
    GENERATE_PUBLIC = 'openssl rsa -outform PEM -pubout'
    ENCODE = 'openssl rsautl -encrypt -inkey %(public)s -pubin'
    DECODE = 'openssl rsautl -decrypt -inkey %(private)s'

##    GENERATE_CERT = 'echo [secret] > %(secret)s'
##    GENERATE_PRIVATE = 'echo [private]'
##    GENERATE_PUBLIC = 'echo [public]'
##    ENCODE = 'echo [encode]  %(public)s'
##    DECODE = 'echo [decode]  %(private)s'

    def __init__(self, name):
        self.name = name
        self.private = None
    def __execute(self, cmd, indata = ''):
        logging.debug("Execute: %s"%cmd)
        p = Popen(cmd, shell = True, stdin=PIPE, stdout=PIPE)
        if indata:
            p.stdin.write(indata)
        return p.stdout.read()
    def generate_cipher(self):
        secret_file = self.name+'_main.pem'
        cmd = KeyBuilder.GENERATE_CERT%{'secret': secret_file}
        self.__execute(cmd)
        cipher = open(secret_file).read()
        return (cipher, secret_file)
    def generate_private(self):
        cmd = KeyBuilder.GENERATE_PRIVATE
        self.private = self.__execute(cmd)
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

        f.write(data)
        f.close()
        cmd = KeyBuilder.DECODE%{'private': keyfile}
        decoded = self.__execute(cmd, data)
        #os.unlink(keyfile)
        return decoded

