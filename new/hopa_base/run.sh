#/bin/bash
if [ "$1" == "k" ]; then
    killall hopa
    exit 0
fi


gcc main.c -o hopa
./hopa  &
sudo ip addr add 10.0.0.1/24 dev tun0
sudo ip link set tun0 up

