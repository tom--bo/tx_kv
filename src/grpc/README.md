# protobuf definitions

## build

```sh
$ protoc -I ./protos --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/kv.proto

$ protoc -I ./protos --cpp_out=. ./protos/kv.proto
```

