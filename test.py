import struct

class KeyServer:
    INIT = 0
    PUBLIC = 1
    CIPHER = 2
    HI = "Slava Ukraini!"
    BY = "Heroyam Slava!"

    def __init__(self):
        self.state = KeyServer.INIT
        self.public_key = None
        self.hello = ''
    def recv(self, data):
##        log("KeyServer received %s bytes"%len(data))

        if(self.state == KeyServer.INIT):
            self.public_key = self.parse_hello(data)
            if self.public_key:
                self.state = KeyServer.PUBLIC
            return True
        elif (self.state == KeyServer.PUBLIC):
            cipher = self.create_cipher()
            encoded = self.encode(cipher, self.public_key)
            self.s.send(encoded)
            self.state = KeyServer.CIPHER
            return True
        else:
            if self.is_ack_valid(data):
                log("Key session done")
            else:
                log("Key session fail!")
            return False
    def parse_size(self, data):
        return (ord(data[0])<<8) | ord(data[1])
    def parse_hello(self, data):
        SIZE_FIELD = 2
        self.hello += data
        if len(self.hello) > len(self.HI) + SIZE_FIELD:
            if self.hello.startswith(self.HI):
                size_field = self.hello[len(self.HI):len(self.HI)+SIZE_FIELD]
                public_key_length = self.parse_size(size_field)
                if len(self.hello) >= len(self.HI) + SIZE_FIELD + public_key_length:
                    public_key = self.hello[len(self.HI) + SIZE_FIELD:]
                    return public_key
        return None
    def encode(self, data, key):
        pass
    def is_ack_valid(self, data):
        return data == self.BY


data = "Slava Ukraini!\x00\x07"+"z"*8


k = KeyServer()

for i in data:
    k.recv(i)

