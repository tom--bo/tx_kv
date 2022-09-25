# tx_kv

Single version, S2PL KVS

## How to build 

```sh
$ cd /path/to/tx_kv
$ mkdir build
$ cd build
$ cmake ..  # This fetch grpc codes via CMake's FetchContent
$ make -j 4
```

## How to run

start kv server

```sh
$ cd /path/to/tx_kv/build
$ ./server  
Server listening on 0.0.0.0:8000
```

start cli-client

```sh
$ cd /path/to/tx_kv/build
$ ./cli  
connection_id: 1
cmd> help
[command list]
 begin
 commit
 rolback (not implemented yet)
 get {key}
 put {key} {val}
 del {key}
 help

cmd> begin
cmd> put 1 100
cmd> commit

cmd> begin
cmd> get 1
100
cmd> commit

cmd> get 1 # auto-commit mode
100

cmd> ^C

```


