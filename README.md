# sqlite3-server

## Build process
1. Install python3 for the python client
2. Install the only dev dependency for the server using `sudo apt install libsqlite3-dev`.
3. Run build to build both client and server

## Usage
After building the project you'll have three binaries in ./bin.
1. `sqlite3-server`. You can execute `./bin/sqlite3-server <port> <dbfile>`. Default port is 8888. `<dbfile>` does not have to exist. It will initialize a new sqlite3 db then.
2. `client`. Example client that connects to http://localhost:8888/. It will execute some concurrent requests to stress test the server a bit. Does not except parameters.
3. `json`. A test application for the json functionality of the server. This is a technical utility. Does not accept parameters. 
