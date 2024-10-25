all: install 

format:
	black .

install:
	-@rm -rf .venv 
	python3 -m venv .venv
	./.venv/bin/pip install -r requirements.txt
	-@rm ./../../../bin/client
	-@ln -s $(pwd)/client ./../../../bin/client

run:
	./.venv/bin/python client.py
