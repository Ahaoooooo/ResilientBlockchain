#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/sha.h>
#include <stack>

#include "./miner.h"
#include "../../data/Blockchain.hpp"
#include "./pow.hpp"

#include "../../verify/verify.h"
#include "../../data/transactions.h"
#include "../../utils/crypto_stuff.h"

using namespace std;

extern mt19937 rng;//std::mt19937是伪随机数产生器，用于产生高性能的随机数。 C++11引入。返回值为unsigned int。
extern tcp_server *ser;
extern boost::thread *mythread;

uint32_t total_mined = 0;


//挖出弱区块
uint32_t mine_weak_block( Blockchain *bc)//只需有两个部分：公共部分、交易部分
{


  	std::unique_lock<std::mutex> l(bc->lock);
	bc->can_write.wait( l, [bc](){return !bc->locker_write;});
	bc->locker_write = true;


	// Concatenate the candidates of all chains 
	vector<string> leaves;		// used in Merkle tree hash computation    vector是可以动态增长的数组

	// Last block of the trailing chain 
	block *trailing_block = bc->get_deepest_child_by_chain_id( 0 );//得到链id为0的链的最后一个区块，赋值给trailing_block
	int trailing_id = 0;
	for( int i=0; i<CHAINS; i++){//CHAINS是configuration中定义的每个节点的链数量
	//遍历该节点的所有链

		block *b = bc->get_deepest_child_by_chain_id( i );//得到i链中最后一个节点
		if( NULL == b ){
			cout <<"Something is wrong in mine_new_block: get_deepest return NULL"<<endl;
			exit(3);//终止程序，返回3
		}
		if( NULL == b->nb ){
			cout <<"Something is wrong in mine_new_block: get_deepest return block with NULL nb pointer"<<endl;
			exit(3);
		}
		if ( b->nb->next_rank > trailing_block->nb->next_rank ){
			trailing_block = b;
			trailing_id = i;
		}//判断此区块的b->nb->next_rank与railing_block->nb->next_ran大小，更新trailing_block和trailing_id

		leaves.push_back( blockhash_to_string( b->hash ) );//将每条链的最后一个区块的hash传入vector<string> leaves
	}//此时trailing_block是该Node中拥有最大next_rank值的block

	// Make a complete binary tree
	uint32_t tot_size_add = (int)pow(2,ceil( log(leaves.size()) / log(2) )) - leaves.size();//Math.pow(底数,几次方)
    /*如：int a=3;
          int b=3;
          int c=(int)Math.pow(a,b);
          就是3的三次方是多少；
		  
		  C 库函数 double ceil(double x) 返回大于或等于 x 的最小的整数值

		  C 库函数 double log(double x) 返回 x 的自然对数
		  
	*/	

	for( int i=0; i<tot_size_add ; i++)
		leaves.push_back( EMPTY_LEAF );//EMPTY_LEAF在params.h中定义#define EMPTY_LEAF "00000000000000000000000000000000"

	// hash to produce the hash of the new block
	string merkle_root_chains = compute_merkle_tree_root( leaves );
    string merkle_root_txs = to_string(rng());//rng是产生随机数，本项目的交易都是模拟的，交易hash用随机数代替
	string h = sha256( merkle_root_chains + merkle_root_txs);

	// Determine the chain where it should go
	uint32_t chain_id = get_chain_id_from_hash(h);//根据新块的hash确定它链到哪条链上

	// Determine the new block，new_block只是新块的hash值
	BlockHash new_block = string_to_blockhash( h );//将string类型的h转换为BlockHash类型


	//Create file holding the whole block
	//Supposedly composed of transactions
	 uint32_t no_txs = create_transaction_block( new_block , ser->get_server_folder()+"/"+blockhash_to_string( new_block ) ); 
	 if( 0 == no_txs  ) {
	 printf("Cannot create the file with transactions\n");
	 fflush(stdout);
	 return 0;
	 }//这里是在测试用例中填充自动生成的假交易


	// Find Merkle path for the winning chain
	vector <string> proof_new_chain = compute_merkle_proof( leaves, chain_id );//chain_id是新块链接的链的链id

	// Last block of the chain where new block will be mined
	block *parent = bc->get_deepest_child_by_chain_id( chain_id );//找到新块链接的链的最后一个区块


	//上面已经计算出了相应的区块属性值，下面直接复制给network_block


	//nb就是新挖出来的区块
	network_block nb;
	nb.chain_id = chain_id;
	nb.parent = parent->hash;
	nb.hash = new_block;
	nb.trailing = trailing_block->hash;
	nb.trailing_id = trailing_id;
	nb.merkle_root_chains = merkle_root_chains;
	nb.merkle_root_txs = merkle_root_txs;
	nb.proof_new_chain = proof_new_chain;
	nb.no_txs = no_txs;
	nb.rank  = parent->nb->next_rank ;
	nb.next_rank  = trailing_block->nb->next_rank;
	if (nb.next_rank <= nb.rank ) nb.next_rank = nb.rank + 1;


	nb.depth = parent->nb->depth + 1;
	unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
	nb.time_mined    = time_of_now;
	nb.time_received = time_of_now;
	for( int j=0; j<NO_T_DISCARDS; j++){//
		nb.time_commited[j] = 0;
		nb.time_partial[j] = 0;
	}

	nb.besited==false;          //将弱块被引用标志位设为false（未被引用）
    //上面的代码是对nb的信息进行更新
	
    //原来代码中用bc->add_block_by_parent_hash_and_chain_id( parent->hash, new_block, chain_id, nb );
	//来产生具体区块block，但是弱区块不用这么复杂，只需产生network_block就行了
	//在这里开始，弱快不需要被添加进bc的链结构中，只需添加进bc的弱区块存储动态数组
	//bc->add(nb);     //这里有问题，问题是a的定义weakblock，而传入的nb是network_block类型

	bc->add_weak_block(chain_id, nb, new_block);
	
	weakblock* wb = bc->find_weak_block_by_hash_and_chain_id(new_block, chain_id);
	if(wb != NULL && wb->nb != NULL){
		wb->is_full_block = true;
		// Increase the miner counter
		bc->add_mined_weak_block();


		// Send the block to peers?????这个地方还需要想想怎么修改，因为我们用nb区块来承载弱区块，
		// 所以没有进入链结构，故向其它节点传播的时候需要额外处理
		//ser->send_block_to_peers( &nb );
		ser->send_weak_block_to_peers( &nb );
		//将弱块发送给其他节点
		//其他节点验证此弱块，并进行同步操作

	}
	


	bc->locker_write = false;
	l.unlock();
	bc->can_write.notify_one();


	return chain_id;
}


//挖出强区块
uint32_t mine_new_block( Blockchain *bc)
{


  	std::unique_lock<std::mutex> l(bc->lock);
	bc->can_write.wait( l, [bc](){return !bc->locker_write;});
	bc->locker_write = true;


	// Concatenate the candidates of all chains 
	vector<string> leaves;		// used in Merkle tree hash computation    vector是可以动态增长的数组

	// Last block of the trailing chain 
	block *trailing_block = bc->get_deepest_child_by_chain_id( 0 );//得到链id为0的链的最后一个区块，赋值给trailing_block
	int trailing_id = 0;
	for( int i=0; i<CHAINS; i++){//CHAINS是configuration中定义的每个节点的链数量
	//遍历该节点的所有链

		block *b = bc->get_deepest_child_by_chain_id( i );//得到i链中最后一个节点
		if( NULL == b ){
			cout <<"Something is wrong in mine_new_block: get_deepest return NULL"<<endl;
			exit(3);//终止程序，返回3
		}
		if( NULL == b->nb ){
			cout <<"Something is wrong in mine_new_block: get_deepest return block with NULL nb pointer"<<endl;
			exit(3);
		}
		if ( b->nb->next_rank > trailing_block->nb->next_rank ){
			trailing_block = b;
			trailing_id = i;
		}//判断此区块的b->nb->next_rank与railing_block->nb->next_ran大小，更新trailing_block和trailing_id

		leaves.push_back( blockhash_to_string( b->hash ) );//将每条链的最后一个区块的hash传入vector<string> leaves
	}//此时trailing_block是该Node中拥有最大next_rank值的block

	// Make a complete binary tree
	uint32_t tot_size_add = (int)pow(2,ceil( log(leaves.size()) / log(2) )) - leaves.size();//Math.pow(底数,几次方)
    /*如：int a=3;
          int b=3;
          int c=(int)Math.pow(a,b);
          就是3的三次方是多少；
		  
		  C 库函数 double ceil(double x) 返回大于或等于 x 的最小的整数值

		  C 库函数 double log(double x) 返回 x 的自然对数
		  
		  */

	for( int i=0; i<tot_size_add ; i++)
		leaves.push_back( EMPTY_LEAF );//EMPTY_LEAF在params.h中定义#define EMPTY_LEAF "00000000000000000000000000000000"

	// hash to produce the hash of the new block
	string merkle_root_chains = compute_merkle_tree_root( leaves );
	string merkle_root_txs = to_string(rng());//rng是产生随机数，本项目的交易都是模拟的，交易hash用随机数代替
	string h = sha256( merkle_root_chains + merkle_root_txs);

	// Determine the chain where it should go
	uint32_t chain_id = get_chain_id_from_hash(h);//根据新块的hash确定它链到哪条链上

	// Determine the new block
	BlockHash new_block = string_to_blockhash( h );//将string类型的h转换为BlockHash类型

	//由于本区块是强区块，所以不需要产生交易部分
	// Create file holding the whole block
	// Supposedly composed of transactions
	uint32_t no_txs = create_transaction_block( new_block , ser->get_server_folder()+"/"+blockhash_to_string( new_block ) ); 
	//if( 0 == no_txs  ) {
	//	printf("Cannot create the file with transactions\n");
	//	fflush(stdout);
	//	return 0;
	//}  


	// Find Merkle path for the winning chain
	vector <string> proof_new_chain = compute_merkle_proof( leaves, chain_id );//chain_id是新块链接的链的链id

	// Last block of the chain where new block will be mined
	block *parent = bc->get_deepest_child_by_chain_id( chain_id );//找到新块链接的链的最后一个区块


	network_block nb;
	nb.chain_id = chain_id;
	nb.parent = parent->hash;
	nb.hash = new_block;
	nb.trailing = trailing_block->hash;
	nb.trailing_id = trailing_id;
	nb.merkle_root_chains = merkle_root_chains;
	nb.merkle_root_txs = merkle_root_txs;
	nb.proof_new_chain = proof_new_chain;
	nb.no_txs = no_txs;
	nb.rank  = parent->nb->next_rank ;
	nb.next_rank  = trailing_block->nb->next_rank;
	if (nb.next_rank <= nb.rank ) nb.next_rank = nb.rank + 1;


	nb.depth = parent->nb->depth + 1;
	unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
	nb.time_mined    = time_of_now;
	nb.time_received = time_of_now;
	for( int j=0; j<NO_T_DISCARDS; j++){//
		nb.time_commited[j] = 0;
		nb.time_partial[j] = 0;
	}
    //上面的代码是对nb的信息进行更新

	//获取unsited weakBlocks
	weakblock* root =  bc->get_weak_block( chain_id );
	vector<BlockHash> unsited_weakblocks;
	bc->get_unsited_blocks(unsited_weakblocks, chain_id, root);
	if(unsited_weakblocks.size() == 0){
		bc->locker_write = false;
		l.unlock();
		bc->can_write.notify_one();
		return chain_id;
	}
    

	// Add the block to the chain
	bc->add_block_by_parent_hash_and_chain_id( parent->hash, new_block, chain_id, nb );
	if( PRINT_MINING_MESSAGES) {
		printf("\033[33;1m[+] Mined block on chain[%d] : [%lx %lx]\n\033[0m", chain_id, parent->hash, new_block);
		fflush(stdout);	
	}//打印挖矿信息，打印链id，parent->hash，新块hash...可以在此处打印强弱块的信息


	// Set block flag as full block
	// Set weak blocks sited by the new block
    block * bz = bc->find_block_by_hash_and_chain_id( new_block, chain_id ); 
	if (NULL != bz && NULL != bz->nb ){
		bz->is_full_block = true;
		bz->number_of_weakblocks = unsited_weakblocks.size();
		bz->vec_weakblocks = (BlockHash*)malloc(sizeof(BlockHash) * unsited_weakblocks.size());
		for(int i = 0; i < unsited_weakblocks.size(); i++){
			bz->vec_weakblocks[i] = unsited_weakblocks[i];
		}
		// Increase the miner counter
		bc->add_mined_block();
	

		// Send the block to peers
		ser->send_block_to_peers( bz );
		//ser->send_block_to_peers( &nb );
	}
	bc->locker_write = false;
	l.unlock();
	bc->can_write.notify_one();


	return chain_id;
}


uint32_t get_mine_time_in_milliseconds( ) 
{

	std::exponential_distribution<double> exp_dist (1.0/EXPECTED_MINE_TIME_IN_MILLISECONDS);
	uint32_t msec = exp_dist(rng);
	//指数分布


	//printf("msec: %0.1f\n", (float)msec/1000);

	if( PRINT_MINING_MESSAGES) {
		printf("\033[33;1m[ ] Will mine new block in  %.3f  seconds \n\033[0m", (float)msec/1000 );
		fflush(stdout);
	}
	
	return msec;
}//挖矿时间也是提前设定的，然后在设定时间上进行指数分布


void miner( Blockchain *bc)
{

	//设定每64个弱块后挖一个强块
	if(total_mined %  3!= 0){
		if (! CAN_INTERRUPT)//Stop the miner even after receiving a block  uint32_t CAN_INTERRUPT = 0;
	   		boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds()/2));
			//睡眠当前线程，睡眠时间为挖矿所需时间
		else{
			try{
		    	boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds()/2));
				//睡眠当前线程，睡眠时间为挖矿所需时间
			}
			catch (boost::thread_interrupted &){
				//处理线程异常，boost::thread_interrupted表示线程收到其它节点传来的new block，该线程被中断
				if( PRINT_INTERRUPT_MESSAGES){
					printf("\033[35;1mInterrupt mining, recieved new block from a peer \n\033[0m");
					fflush(stdout);
				}
				miner( bc );
			}
		}
			
		if( total_mined >= MAX_MINE_BLOCKS) return;
		total_mined++;
		if ( NULL != ser){
			 mine_weak_block(bc);
		}
	}
	else{
		if (! CAN_INTERRUPT)//Stop the miner even after receiving a block  uint32_t CAN_INTERRUPT = 0;
	   		boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds()));
			//睡眠当前线程，睡眠时间为挖矿所需时间
		else{
			try{
		    	boost::this_thread::sleep(boost::posix_time::milliseconds(get_mine_time_in_milliseconds()));
				//睡眠当前线程，睡眠时间为挖矿所需时间
			}
			catch (boost::thread_interrupted &){
				//处理线程异常，boost::thread_interrupted表示线程收到其它节点传来的new block，该线程被中断
				if( PRINT_INTERRUPT_MESSAGES){
					printf("\033[35;1mInterrupt mining, recieved new block from a peer \n\033[0m");
					fflush(stdout);
				}
				miner( bc );
			}
		}
			
		if( total_mined >= MAX_MINE_BLOCKS) return;
		total_mined++;

		if ( NULL != ser){
			mine_new_block(bc);
			//mine_weak_block(bc);
		}
	}

		  

  	// Mine next block
    miner( bc );
	//接着下一轮挖矿.
}


