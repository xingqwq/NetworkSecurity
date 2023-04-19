import socket
import argparse
import random
import hashlib
import time
from des import DESModel

def certCodeGeneration(dataLen):
    data = ""
    for i in range(0,dataLen):
        tmp = random.randint(1,3)
        if tmp == 1:
            data += chr(ord('A')+random.randint(0,25))
        elif tmp == 2:
            data += chr(ord('a')+random.randint(0,25))
        else:
            data += chr(ord('0')+random.randint(0,9))
    return data

# 参数
parse = argparse.ArgumentParser()
parse.add_argument("--port",type=int,default=6689)
parse.add_argument("--addr",type=str,default="127.0.0.1")
parse.add_argument("--data_len",type=int,default=1024)
args = parse.parse_args()

# 服务器连接
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serverSocket.connect((args.addr, args.port))
print("已成功连接上服务器")

# 获取用户名、口令
userName = input("请输入登录用户名: ")
password = input("请输入登录口令: ")

# 生成随机认证码
certCode = certCodeGeneration(16)
print("所生成的随机认证码为： {}".format(certCode))

# 计算散列值1
hash1 = hashlib.md5((userName+password).encode("utf-8"))
hash1Str = hash1.hexdigest()

# 计算散列值2
hash2 = hashlib.md5((hash1Str+certCode).encode("utf-8"))
hash2Str = hash2.hexdigest()

print(len(hash1Str.encode("utf-8")))

print("所生成的散列值1为 {} \n所生成的散列值2为 {}".format(hash1Str, hash2Str))

# 将用户名，散列值2，认证码明文传送到服务器端
sendData = userName+" "+hash2Str+" "+certCode
sendData = sendData.encode("utf-8")
serverSocket.send(b'000'+sendData)

# 接收服务端发送来的数据
recvData = serverSocket.recv(100)
if recvData[0:3] == b'001':
    desC = DESModel()
    recvStr = desC.decrypt(recvData[3:], hash1Str[0:8])
    with open("./{}_{}.txt".format(userName, int(time.time())),"w") as file:
        file.writelines(recvStr+"\n")
else:
    print("登陆失败，请检查密码")
    exit(0)
    
# 修改密码
isSet = input("是否需要修改数据[0不修改，1修改]：")
if isSet == "0":
    print("已正确接收，再见")
else:
    serverSocket.send("002 SET NEWPASS.".encode("utf-8"))
    recvData = recvData = serverSocket.recv(100)
    if recvData[0:3] == b'001':
        newPass = input("请输入新的密码: ")
        # 生成随机认证码
        certCode = certCodeGeneration(16)
        print("所生成的随机认证码为： {}".format(certCode))

        # 计算散列值1
        hash1 = hashlib.md5((userName+newPass).encode("utf-8"))
        hash1Str = hash1.hexdigest()

        # 计算散列值2
        hash2 = hashlib.md5((hash1Str+certCode).encode("utf-8"))
        hash2Str = hash2.hexdigest()

        print(len(hash1Str.encode("utf-8")))

        print("所生成的散列值1为 {} \n所生成的散列值2为 {}".format(hash1Str, hash2Str))

        # 将用户名，散列值1，认证码明文传送到服务器端
        sendData = userName+" "+hash1Str+" "+certCode
        sendData = sendData.encode("utf-8")
        serverSocket.send(sendData)
        
        # 接收服务端发送来的数据
        recvData = serverSocket.recv(100)
        if recvData[0:3] == b'001':
            desC = DESModel()
            recvStr = desC.decrypt(recvData[3:], hash1Str[0:8])
            with open("./{}_{}.txt".format(userName, int(time.time())),"w") as file:
                file.writelines(recvStr+"\n")
            print("密码修改成功")
        else:
            print("修改密码失败")
            exit(0)
        
    else:
        print("服务端拒绝修改密码")