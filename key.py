from subprocess import Popen


class Secret:
    HI = "Slava Ukraini!"
    BY = "Heroyam Slava!"


class KeyBuilder:
    GENERATE_CERT = 'openssl genrsa -out %(secret)s 1024'
    GENERATE_PRIVATE = 'openssl genrsa 1024'
    GENERATE_PUBLIC = 'openssl rsa -outform PEM -pubout'
    ENCODE = 'openssl rsautl -encrypt -inkey %(public)s -pubin'
    DECODE = 'openssl rsautl -decrypt -inkey %(private)s'
    def __init__(self, name):
        self.name = name
        self.private = None
    def __execute(self, cmd, indata = ''):
        print(cmd)
        p = Popen(cmd.split(), stdin=PIPE, stdout=PIPE, close_fds=True)
        if indata:
            p.stdin.write(indata)
        return p.stdout.read()
    def generate_cipher(self):
        secret_file = self.name+'_main.pem'
        cmd = key.GENERATE_CERT%{'secret': secret_file}
        self.__execute(cmd)
        cipher = open(secret_file).read()
        return (cipher, secret_file)
    def generate_private(self):
        cmd = key.GENERATE_PRIVATE
        self.private = self.__execute(cmd)
    def generate_public(self):
        cmd = key.GENERATE_PUBLIC
        return self.__execute(cmd, self.private)
    def encode(self, key = None, data = None):
        keyfile = self.name+'_pub.pem'
        f = open(key, 'w')
        f.write(key)
        f.close()
        cmd = key.ENCODE%{'public': keyfile}
        encoded = self.__execute(cmd, data)
        os.unlink(keyfile)
        return encoded
    def decode(self, data):
        keyfile = self.name+'_pri.pem'
        f = open(key, 'w')
        f.write(key)
        f.close()
        cmd = key.DECODE%{'private': key}
        decoced = self.__execute(cmd, data)
        os.unlink(keyfile)
        return decoded



