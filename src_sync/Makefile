all: gen grpc cl sr


gen:
	protoc -I ../protos/ --cpp_out=.  ../protos/BalanceStorageRPC.proto
	protoc -I ../protos/ --grpc_out=.  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ../protos/BalanceStorageRPC.proto


cl:
	g++ -I. client.cpp -c
	g++ *.pb.o client.o -o client `pkg-config --libs protobuf grpc++`

sr:
	g++ -I. server.cpp -c
	g++ *.pb.o server.o -o server `pkg-config --libs protobuf grpc++`


grpc: ./BalanceStorageRPC.grpc.pb.cc ./BalanceStorageRPC.pb.cc
	g++ ./BalanceStorageRPC.grpc.pb.cc -c
	g++ ./BalanceStorageRPC.pb.cc -c


clean:
	rm -f *.o client server *.pb.cc *.pb.h 

