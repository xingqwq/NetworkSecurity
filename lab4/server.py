import socket
import argparse
import threading
import time
import pymysql
import hashlib
from des import DESModel

class Server:
    def __init__(self,args):
        self.args = args
        self. db = pymysql.connect(
            host="localhost",
            port=3306,
            user='root',
            password='',
            charset='utf8mb4'
        )
        
    def initListener(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((args.addr, args.port))
        server_socket.listen(10)
        print("等待成员的加入...")
        while True:
            try:
                new_socket, client_addr = server_socket.accept()
                newThread = threading.Thread(target=self.solveThread,args=(new_socket,))
                newThread.start()   
            except BlockingIOError:
                continue
    
    def updateHash(self, userName, hash):
        # 从数据库中查找
        cursor = self.db.cursor()
        cursor.execute("use netlab4")
        sql = "update user set hash = '{}' where name = '{}'".format(hash, userName)
        if not cursor.execute(sql):
            cursor.close()
            return "Not Update"
        else:
            cursor.close()
            return "Update"
    
    def getUserHash(self, userName):
        # 从数据库中查找
        cursor = self.db.cursor()
        cursor.execute("use netlab4")
        sql = "select hash from user where name = '{}'".format(userName)
        if not cursor.execute(sql):
            cursor.close()
            return "Not Found"
        else:
            hash = cursor.fetchone()[0]
            cursor.close()
            return hash
        
    def solveThread(self,client):
        while True:
            data = client.recv(100)
            if data[0:3] == b'000':
                data = data[3:]
                data = data.decode("utf-8") 
                data = data.split(" ")
                hashFromSql = self.getUserHash(data[0])
                if hashFromSql == "Not Found":
                    print("当前用户 {} 未进入数据库".format(data[0]))
                else:
                    tmpHash = hashlib.md5((hashFromSql+data[2]).encode("utf-8"))
                    if tmpHash.hexdigest() == data[1]:
                        print("当前用户 {} 通过认证".format(data[0]))
                        desC = DESModel()
                        enCode = desC.encrypt("wolaile", hashFromSql[0:8])
                        client.send(b'001'+enCode)
                    else:
                        print("当前用户 {} 认证失败".format(data[0]))
                        client.send(b'000000')
            elif data[0:3] == b'002':
                client.send(b'001000')
                data = client.recv(100)
                data = data.decode("utf-8") 
                data = data.split(" ")
                hashFromSql = self.updateHash(data[0], data[1])
                if hashFromSql == "Not Update":
                    client.send(b'000000')
                    print("当前用户 {} 更新密码失败".format(data[0]))
                else:
                    print("当前用户 {} 更新密码成功".format(data[0]))
                    desC = DESModel()
                    enCode = desC.encrypt("wolaile", data[1][0:8])
                    print(enCode)
                    client.send(b'001'+enCode)


if __name__== "__main__" :
    parse = argparse.ArgumentParser()
    parse.add_argument("--port",type=int,default=6689)
    parse.add_argument("--addr",type=str,default="127.0.0.1")
    parse.add_argument("--data_len",type=int,default=1024)
    args = parse.parse_args()
    
    runner = Server(args)
    runner.initListener()