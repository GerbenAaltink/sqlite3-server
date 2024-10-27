all: build run


build:
	-@mkdir -p ./bin
	gcc -Ofast -Werror -Wextra -Wall  ranku.c -o ./bin/ranku -lsqlite3

test: test_json

test_json:
	-@mkdir -p ./bin
	gcc -Ofast -Werror -Wextra -Wall json.c -o ./bin/json 
	./bin/json

format:
	clang-format *.c *.h -i

run:
	./bin/ranku 8888 ./local.db