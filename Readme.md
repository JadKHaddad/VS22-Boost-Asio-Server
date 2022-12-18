# VS 22

## Description
* The server will listen on the port specified as the first argument and will accept connections asynchonously from clients.
* The connections will be stored and held open until the client sends an exit message. The server will then close and drop the connection.
* The server will accept a maximum of 10 clients. Changing this value is as simple as changing the ```MAX_CLIENTS``` constant in ```helper.hpp```.
* When every client has connected to the server, the main loop of the game will start in a separate thread.
* Every connection from new clients will be refused after the game has started.
* The game will run until the last client disconnects.
* The server uses ```Boost.Asio``` to handle the asynchronous connections and ```Ncurses``` to display the game state.
* Please make sure to run the server in a terminal that supports colors and is big enough to display the game state.
* For the clients to connect to the server, the host and the port must be specified as the first and second argument respectively.
* A client can exit the game at any time by pressing ```ctrl+c```. The server will then send the score, close the connection and drop the client. If the server did not respond, the client can exit by pressing ```ctrl+c``` again.


## Compilation

### Development
The server and the client can be compiled without optimizations by typing
```sh
make

# or

make all
```
in the ```app``` directory.

### Release

The server and the client can be compiled with optimizations and static linking of the Boost, Ncurses and Pthread libraries by typing
```sh
make release
```
in the ```app``` directory.

## Execution
### Server
The server can be executed by typing
```sh
./server <port>
```

### Client
The client can be executed by typing
```sh
./client <host> <port>
```

## Docker

You can also build and run the server and the client in a docker container. In the ```root``` directory, build the image by typing

```sh
docker build -t debian-boost-ncurses-release .
```

Run the server by typing
```sh
docker run -it --rm --name debian-boost-ncurses-release --entrypoint /home/app/server debian-boost-ncurses-release 80
```
Make sure the server is running before you start the clients. You can run a client by typing

```sh
docker exec -it debian-boost-ncurses-release /home/app/client 127.0.0.1 80
```
## Other versions
* [Rust](https://github.com/JadKHaddad/VS22-Boost-Asio-Server-Rewritten-In-Rust)