all: test build 

build: server_build client_build
	@echo "build done"
	du bin/* -sch

ensure_bin:
	-@mkdir -p ./bin

server_build: ensure_bin
	cd ./src/server && make build

server_run:
	./bin/sqlite3-server 8888 local.db

client_build: ensure_bin
	cd ./src/client/python && $(MAKE) install

client_run: 
	./bin/client 

utro:
	./src/client/python/.venv/bin/utro http://127.0.0.1:8888

utro_history:
	./src/client/python/.venv/bin/utro http://127.0.0.1:8888 --history --data=abc,def,ghi

test: ensure_bin
	cd ./src/server && make test


zip:
	-@rm ./sqlite3-server.zip
	cd ./src/server && make format
	zip  -r ./sqlite3-server.zip -x=src/client/python/.* -x=src/client/python/__*  ./src ./.gitignore 

tar:
	-@rm ./sqlite3-server.zip
	cd ./src/server && make format
	tar -czvf sqlite3-server.tar.gz  --exclude=src/client/python/.* --exclude=src/client/python/__*  ./src ./.gitignore 