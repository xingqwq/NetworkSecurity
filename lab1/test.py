import socket
import argparse
import sys
import os
import math
import threading
import time
import pickle

class Server:
    def __init__(self,args):
        self.args = args
        self.recvLock = threading.Lock()
        self.sendLock = threading.Lock()
        self.socketLock = threading.Lock()
        self.waitinStatus = 1
        self.runStatus = 1
        self.client = []
        self.recvClient = []
        self.sendQueue = []
        
    def initListener(self):
        try:
            waitJoin = threading.Thread(target=self.waitJoining)
            waitJoin.start()
        except KeyboardInterrupt:
            self.runStatus = 0
            self.waitinStatus = 0
        
    def waitJoining(self):    
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind((args.addr, args.port))
        server_socket.listen(10)
        print("等待成员的加入...")
        while self.waitinStatus:
            try:
                new_socket, client_addr = server_socket.accept()
                newThread = threading.Thread(target=self.judgeJoining,args=(new_socket,))
                newThread.start()   
            except BlockingIOError:
                continue
        print("等待成员加入已关闭")
        server_socket.close()

    def judgeJoining(self,socketPtr):
        print("接收到一个连接，正在处理")
        time.sleep(0.01)
        data = socketPtr.recv(args.data_len)
        data = data.decode().strip(b'\x00'.decode())
        print(data,type(data))
        if data[0] == "1":
            socketPtr.send("Recv.".encode())
            data = socketPtr.recv(args.data_len)
            data = data.decode().strip(b'\x00'.decode())
            print(data,len(data))
            if len(data) <2:
                return
            print("send Init")
            socketPtr.send("Recv.".encode())
            print("send Success")
            f = open("tmp-"+data,"wb");
            while True:
                data = socketPtr.recv(args.data_len)
                if data[0:8] == b'Finish.':
                    break
                else:
                    f.write(data)
            f.close()
            print("已完成文件传输\n")
        elif data[0] == "2":
            path = '/home/xingqwq/下载/'
            filePathStr = ''
            for file_name in os.listdir(path):
                if len(filePathStr) + len(str(file_name)+'\n')>500:
                    break
                filePathStr+=(str(file_name)+'\n')
            socketPtr.send(filePathStr.encode())
            print("已发送文件列表",len(filePathStr.encode()))
            data = socketPtr.recv(args.data_len)
            data = data.decode().strip(b'\x00'.decode())
            print(data)
            f = open(path+data,"rb")
            sendData = b''
            while True:
                tmpi = f.read(100)
                if len(tmpi) == 0:
                    break
                sendData += tmpi
                if len(sendData) >= 1000:
                    socketPtr.send(sendData)
                    sendData = b''
                    print("已发送一次数据")
            if len(sendData) != 0:
                socketPtr.send(sendData)
            time.sleep(1)
            socketPtr.send("Finish.".encode())
            print("已完成文件发送")
                

if __name__== "__main__" :
    parse = argparse.ArgumentParser()
    parse.add_argument("--job",type=str,default="server")
    parse.add_argument("--port",type=int,default=6689)
    parse.add_argument("--addr",type=str,default="127.0.0.1")
    parse.add_argument("--data_len",type=int,default=1024)
    parse.add_argument("--car_num",type=int,default=4)
    args = parse.parse_args()
    runner = Server(args)
    runner.initListener()