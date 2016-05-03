#/bin/bash

gcc -o main UdpServer.cpp main.cpp -lstdc++ -lpulse-simple -lpulse -lfftw3 -lm -lpthread -lmgl -lmgl-qt 