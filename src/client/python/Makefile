all: install 

format:
	black .

install:
	-@rm -rf .venv 
	python3 -m venv .venv
	. ./.venv/bin/activate && cd ./utro && $(MAKE) build
	./.venv/bin/pip install -e utro 
	-@rm ./../../../bin/client
	@echo "#!$(CURDIR)/.venv/bin/python $(CURDIR)/client.py" > ./../../../bin/client
	@chmod +x ./../../../bin/client

run:
	./.venv/bin/python client.py
