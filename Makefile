build: 
	g++ -o client client.cpp helpers.cpp buffer.cpp -Wall
run: 
	./client

clean:
	rm -f *.o client
