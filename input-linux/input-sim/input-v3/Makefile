CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -O2 -pthread
all: kernel_sim_server ev_client

kernel_sim_server: kernel_sim_server.cpp
	$(CXX) $(CXXFLAGS) kernel_sim_server.cpp -o kernel_sim_server

ev_client: ev_client.c
	$(CC) -O2 ev_client.c -o ev_client

clean:
	rm -f kernel_sim_server ev_client /tmp/inputsim.sock

