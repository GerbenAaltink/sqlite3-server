all: test build 

build: server_build client_build
	@echo "build done"
	du bin/* -sch

ensure_bin:
	-@mkdir -p ./bin

server_build: ensure_bin
	cd ./src/server && make build

server_run:
	./bin/sqlite3-server

client_build: ensure_bin
	cd ./src/client/python && $(MAKE) install

client_run: 
	./bin/client 

test: ensure_bin
	cd ./src/server && make test

zip:
	-@rm ./sqlite3-server.zip
	cd ./src/server && make format
	zip  -r ./sqlite3-server.zip -x=src/client/python/.* -x=src/client/python/__*  ./src ./.gitignore 