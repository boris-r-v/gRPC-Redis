#!/bin/bash


cd src_sync/protos
protoc -I ../../protos/  --cpp_out=. ../../protos/BalanceStorageRPC.proto
protoc -I ../../protos/  --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ../../protos/BalanceStorageRPC.proto

