import logging
from key import KeyBuilder, Secret



class KeyClient:
    BUFFER_SIZE = 16384
    WAIT_HI = 0
    WAIT_BY = 1

    def __init__(self, ip, port):
        self.ip, self.port = (ip, port)
        self.key_builder = KeyBuilder('')
        self.key_builder.generate_private()
    def init(self):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((self.ip, int(self.port)))
        logging.debug("Client connected")
    def run(self):
        self.init()

        data = self.create_hi()
        self.s.send(data)

        data = self.s.recv(BUFFER_SIZE)
        if not data:
            logging.error('Dropped')
            return
        cipher = self.parse_cipher(data)
        if cipher:
            self.save(cipher)
            self.s.send(Secret.BY)
        else:
            logging.error('No secret!')
        self.s.close()
    def serialize_size(self, size):
        return chr(size>>8)+chr(size&0xFF)
    def create_hi(self):
        data = Secret.HI
        public = self.key_builder.generate_public()
        data += self.serialize_size(len(public))
        data += public
        return data
    def parse_cipher(self, data):
        return self.key_builder.decode(data)
    def save(self, cipher):
        f = open('cert.pem')
        f.write(cipher)
        f.close()




if len(sys.argv) >= 2:
    logging.basicConfig(level=logging.DEBUG,
                            format='%(asctime)s %(message)s',
                            filename='cli_sec.log',
                            filemode='a')

    server = sys.argv[1]
    KeyClient().run(server.split(':'))
else:
    print("USAGE: serverip:port")
