# cs798-Advanced-Distributed-System
## Project 1

### Part 1

compile two c++ file with following commands, requiring changing server's ip and uncommenting.

​	gcc client.cpp -o client

​	gcc server.cpp -o server

### Part 2 ###

put .proto file under ./examples/protos/, .cc file under ./examples/cpp/helloworld (since I dont want to write makefile). complile it with "make", requiring changing server's ip and uncommenting. 

### Part 3 ###

Using .jar to provide environment for compiling. First run:

​	export CP=.:amqp-client-5.5.1.jar:slf4j-api-1.7.21.jar:slf4j-simple-1.7.22.jar

Then compile RPCServer and RPCClient:

​	javac -cp $CP RPCServer.java

​	javac -cp $CP RPCClient.java

