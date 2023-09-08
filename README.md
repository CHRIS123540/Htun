# Htun
vpn
该项目是利用tun虚拟网卡实现的一个数据包截获

一号机为client 二号机为server

# 服务器端

`cd /home/hlx`

`gcc prox.c -o prox`

`./prox -i tun0 -s -d`

## 再开一个shell

`sudo ip addr add 10.0.0.1/24 dev tun0`

`sudo ip link set tun0 up`

`iperf -s`

# 客户端
`cd /home/hlx`

`gcc prox.c -o prox`

`./prox -i tun0 -c 192.168.0.213 -d`


## 再开一个shell

`sudo ip addr add 10.0.0.2/24 dev tun0`

`sudo ip link set tun0 up`

`iperf -c 10.0.0.1 -t 10 -i 1`
# 参考资料

https://github.com/gregnietsky/simpletun

https://paper.seebug.org/1648/

https://ctimbai.github.io/2019/03/01/tech/net/vnet/%E5%9F%BA%E4%BA%8Etaptun%E5%86%99%E4%B8%80%E4%B8%AAICMP%E7%A8%8B%E5%BA%8F/
