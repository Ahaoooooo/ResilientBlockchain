#	g++ -O3 -std=c++11 node.cpp ./consensus/raft/miner.cpp ./verify/verify.cpp ./consensus/raft/raft.cpp ./data/Blockchain.cpp ./data/transactions.cpp ./utils/crypto_stuff.cpp ./network/get_ip.cpp ./network/requests.cpp ./network/process_buffer.cpp ./utils/misc.cpp ./config/configuration.cpp -lboost_system -lboost_thread -lboost_chrono -lpthread -lboost_filesystem -lssl -lcrypto   -o Node

all:
	g++ -O3 -std=c++11 node.cpp ./consensus/pow/miner.cpp ./verify/verify.cpp ./consensus/pow/pow.cpp ./data/Blockchain.cpp ./data/transactions.cpp ./utils/crypto_stuff.cpp ./network/get_ip.cpp ./network/requests.cpp ./network/process_buffer.cpp ./utils/misc.cpp ./config/configuration.cpp -lboost_system -lboost_thread -lboost_chrono -lpthread -lboost_filesystem -lssl -lcrypto   -o Node

