all: build run

build:
	gcc -Ofast -Werror -Wextra -Wall  sqlite3-server.c -o ../../bin/sqlite3-server -lsqlite3 `pkg-config sqlite3 --cflags --libs`

test: test_json

test_json:
	gcc -Ofast -Werror -Wextra -Wall json.c -o ../../bin/json 
	./../../bin/json

format:
	clang-format *.c *.h -i

run:
	../../bin/sqlite3-server 8888 ../../local.db