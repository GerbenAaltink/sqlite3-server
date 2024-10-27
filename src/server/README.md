# Ranku SQLite 3 REST server

# Introduction
Ranku is a SQLite 3 REST server. It allows query execution with parameters using REST. To see how that works, look at my other repository "Utro" where the client is located.

## Build instructions

```
sudo apt-get install libsqlite3-dev
make build
```

## Usage instructions
After building the binary utro is created in `./bin`. You can run it by executing `./bin/ranku <port> <db/file/path.db>`. Database file will just be created if it doesn't exists yet.
