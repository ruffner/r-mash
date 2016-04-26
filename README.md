# r-mash
A simple, unsecured, restricted remote shell. Created for systems programming, CS485G, at the University of Kentucky.


### Use
To build, run ```make```. 

The server: ```./rrsh-server <port>```. Where ```<port>``` is greater than 1023. 

The client: ```./rrsh-client <hostname> <port>```. Where ```<hostname>``` and ```<port>``` refer to the location of the running ```rrsh-server```.

### ToDo
- Imlement ```select()``` to handle multiple spawned connections.
- Occasionally first line of command output is clobbered

### Provided files
- ```csapp.h```: I/O wrappers
- ```csapp.c```: I/O wappers
- ```parser.c```: command string parser

### Modified files
- ```rrsh.h``` - edited from ```shell.h```, part of the mash shell programming assignment. Defines added.

### Original Files
- ```RRSHServerSession.cc```: Class implementation for handling an ```accept()```ed connection.
- ```RRSHServerSession.h```: Class definition and user struct.
- ```rrsh-server.cc```: Handles 1 active connection (at a time) and passes it to a new RRSHServerSession object.
- ```rrsh-client.cc```: Connects to the given address and port and attempts authentication. If allowed, gives shell prompt for remote commands to be entered.