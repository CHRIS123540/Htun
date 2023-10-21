#/bin/bash
if [ "$1" == "k" ]; then
    killall hopa
    exit 0
fi

make
./hopa  &
sudo ip addr add 10.0.0.2/24 dev tun0
sudo ip link set tun0 up
