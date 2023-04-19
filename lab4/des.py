from Crypto.Cipher import DES
from binascii import b2a_hex, a2b_hex

class DESModel:
    def __init__(self):
        self.mode = DES.MODE_CBC

    def encrypt(self, text, key):
        try:
            text = text.encode('utf-8')
            cryptor = DES.new(key.encode("utf-8"), self.mode, key.encode("utf-8"))
            length = 16
            count = len(text)
            if count < length:
                add = (length - count)
                text = text + ('\0' * add).encode('utf-8')
            elif count > length:
                add = (length - (count % length))
                text = text + ('\0' * add).encode('utf-8')
            self.ciphertext = cryptor.encrypt(text)
            return b2a_hex(self.ciphertext)
        except:
            return ""

    def decrypt(self, text, key):
        try:
            cryptor = DES.new(key.encode('utf-8'), self.mode, key.encode('utf-8'))
            plain_text = cryptor.decrypt(a2b_hex(text))
            # return plain_text.rstrip('\0')
            return bytes.decode(plain_text).rstrip('\0')
        except:
            return ""