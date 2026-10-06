sudo docker run --rm -v "$PWD:/work" -w /work rk3566-builder \aarch64-linux-gnu-g++ -Wall -O2 -o test_2004 a.cpp
scp build-arm20/test root@192.168.137.100:~/