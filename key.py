

class KeyBuilder:
    GENERATE_CERT = 'openssl genrsa -out %(private)s 1024'
    GENERATE_PRIVATE = 'openssl genrsa -out %(private)s 1024'
    GENERATE_PUBLIC = 'openssl rsa -in %(private)s -out %(public)s -outform PEM -pubout'
    ENCODE = 'openssl rsautl -encrypt -inkey %(public)s -pubin -in %(infile)s -out %(infile)s'
    DECODE = 'openssl rsautl -decrypt -inkey %(private)s -in %(infile)s -out %(infile)s'
    def __init__(self, name):
        self.name = name
    def cleanup(self):
        #delete all keys excpet final CERT
        pass
    def __execute(self, cmd):
        print(cmd)
        os.system(cmd)
    def generate_cipher(self):
        cmd = key.GENERATE_CERT%{'private': self.name+'_main.pem'}
        self.__execute(cmd)
        cipher = open(self.name+'_main.pem').read()
        return (cipher, self.name+'_main.pem')
    def generate_private(self):
        cmd = key.GENERATE_PRIVATE%{'private': self.name+'_pri.pem'}
        self.__execute(cmd)
    def generate_public(self):
        cmd = key.GENERATE_PUBLIC%{'private': self.name+'_pri.pem', 'public': self.name+'_pub.pem'}
        self.__execute(cmd)
    def encode(self, key = None, data = None):
        if key:
            f = open(self.name+'_pub.pem', 'w')
            f.write(key)
            f.close()
        if data:
            f = open(self.name+'.in', 'w')
            f.write(data)
            f.close()
        cmd = key.ENCODE%{'public': self.name+'_pub.pem', 'infile': self.name+'.in', 'infile': self.name+'.out'}
        self.__execute(cmd)
        if data:
            return open(self.name+'.out').read()
    def decode(self):
        cmd = key.DECODE%{'private': self.name+'_pri.pem', 'infile': self.name+'.in', 'infile': self.name+'.out'}
        self.__execute(cmd)

