all: test build 

build: server_build client_build
	@echo "build done"
	du bin/* -sch

server_build: 
	cd ./src/server && make build

server_run:
	./src/server/bin/ranku 8887 local.db

server_run_verbose:
	./src/server/bin/ranku 8888 local.db --verbose

client_build: 
	-@mkdir -p ./bin
	cd ./src/client/python && $(MAKE) install

client_run: 
	./bin/client 

utro:
	./src/client/python/.venv/bin/utro http://127.0.0.1:8888

utro_history:
	./src/client/python/.venv/bin/utro http://127.0.0.1:8888 --history --data=abc,def,ghi

test: 
	cd ./src/server && make test


zip:
	-@rm ./sqlite3-server.zip
	cd ./src/server && make format
	zip  -r ./sqlite3-server.zip -x=src/client/python/.* -x=src/client/python/__* -x=src/client/python/ranku/.*  ./src ./.gitignore 

tar:
	-@rm ./sqlite3-server.zip
	cd ./src/server && make format
	tar -czvf sqlite3-server.tar.gz  --exclude=src/client/python/.* --exclude=src/client/python/ranku/.* --exclude=src/client/python/__*  ./src ./.gitignore 
