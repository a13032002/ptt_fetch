v1 : v1.cpp bbs remote
	g++ v1.cpp bbs.o remote.o -o v1.o -pthread 

bbs : bbs.cpp
	g++ -c bbs.cpp  -o bbs.o -D REMOTE_OUTPUT


remote : remote.cpp
	g++ -c remote.cpp -o remote.o 

run_v1 : v1
	rm -f r_i r_o
	./v1.o

remote_input : remote_input.cpp
	g++ remote_input.cpp -o remote_input.o

remote_output : remote_output.cpp
	g++ remote_output.cpp -o remote_output.o

clean :
	rm -f *.o
