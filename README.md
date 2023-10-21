# Htun
vpn 隧道
该项目是利用tun虚拟网卡实现的一个数据包截获
new文件夹中两个文件服务端客户端两个版本，拆分了底层收发包过程，利用多线程使整体逻辑更加清晰。

10.21最新更新：上传两个新的文件夹，实现选路的客户端和服务器端


# 服务器端


`gcc server.c -o test -lpthread`

`./test  &`

`sudo ip addr add 10.0.0.1/24 dev tun0`

`sudo ip link set tun0 up`

## 再开一个shell

`ping 10.0.0.2`

# 客户端

`gcc client.c -o test -lpthread`

`./test  &`

`sudo ip addr add 10.0.0.2/24 dev tun0`

`sudo ip link set tun0 up`

## 再开一个shell

`ping 10.0.0.1`
# 参考资料

https://github.com/gregnietsky/simpletun

https://paper.seebug.org/1648/

https://ctimbai.github.io/2019/03/01/tech/net/vnet/%E5%9F%BA%E4%BA%8Etaptun%E5%86%99%E4%B8%80%E4%B8%AAICMP%E7%A8%8B%E5%BA%8F/
