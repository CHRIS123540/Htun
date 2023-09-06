# Htun
vpn

# 服务器端

`cd /home/hlx`
`gcc prox.c -o prox`


再开一个shell
sudo ip addr add 10.0.0.1/24 dev tun0
sudo ip link set tun0 up

# 客户端
`cd /home/hlx
gcc
`

再开一个shell
sudo ip addr add 10.0.0.2/24 dev tun0
sudo ip link set tun0 up

# 参考资料

https://github.com/gregnietsky/simpletun

https://paper.seebug.org/1648/

https://ctimbai.github.io/2019/03/01/tech/net/vnet/%E5%9F%BA%E4%BA%8Etaptun%E5%86%99%E4%B8%80%E4%B8%AAICMP%E7%A8%8B%E5%BA%8F/
