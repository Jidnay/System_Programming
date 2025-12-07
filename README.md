# System Programming Project: 

This project implements a **multi-client, single-server** mini game using system programming concepts.

## ⚙️ Building the Executables

Use the `make` utility in the project root directory to compile both the server and client programs. This command will generate the necessary executables in their respective subdirectories.

```bash
make
```

## Starting the Server
The server must be started first. It will listen on a designated port for incoming client connections.

1. Navigate to Server directory:
```bash
cd Server
```

2. Execute the server program
```bash
./server
```

## Starting the Client(s)
Clients can be started after the server is running. You can run multiple instances of the client program simultaneously in different terminals.

1. Navigate to the Client directory (in a new terminal window/tab):
```bash
cd Client
```

2. Execute the client program:
```bash
./client
```

## Project Result
