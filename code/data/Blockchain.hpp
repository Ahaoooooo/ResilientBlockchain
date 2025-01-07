#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include  <condition_variable>
#include "../config/params.h"

using namespace std;

extern string my_ip;
extern uint32_t my_port; 



typedef uint64_t BlockHash;



/*
 *
 * 'block' is one node in the tree chain 
 *
 */

//定义区块的网络属性
typedef struct networkblocks{
 	uint32_t chain_id;                //表明该区块是哪条链
 	BlockHash parent;                 //父区块的hash
 	BlockHash hash;                   //本区块的hash
	BlockHash trailing;
	uint32_t trailing_id;
	string merkle_root_chains;       
	string merkle_root_txs;               //交易信息
	vector <string> proof_new_chain;
	uint32_t no_txs;                      //交易信息
	uint32_t depth;
	uint32_t rank;
	uint32_t next_rank;
	bool besited;                         //在该块是弱块的情况下，表明该块是否被引用
	unsigned long time_mined;
	unsigned long time_received;
	unsigned long time_commited[NO_T_DISCARDS];
	unsigned long time_partial[NO_T_DISCARDS];//上述time_commited和time_partial都是类似网络延迟参数指标
}network_block;





//定义一个弱区块
typedef struct weakhashes{
	BlockHash hash;
	network_block *nb;
	bool is_full_block;//该属性表示该区块是不是完整区块，初始化好的区块但没经过pow的是不完整区块，经过pow的区块是完整区块
	//left等都是区块
	struct weakhashes *left, *right;
	//struct weakhashes *parent;
	//struct weakhashes *child;
	//struct weakhashes *sibling;
}weakblock;
//在block类中定义left，right等block结构体指针的目的是为了使区块们之间形成一定的逻辑结构（目前初步判断这个结构是类似于二叉排序树的结构）





//定义一个强区块（定义区块的简单属性）
typedef struct hashes{
	BlockHash hash;
	network_block *nb;
	bool is_full_block;//该属性表示该区块是不是完整区块，初始化好的区块但没经过pow的是不完整区块，经过pow的区块是完整区块
	uint32_t number_of_weakblocks;
    BlockHash* vec_weakblocks;//在强区块中加入一个vector数组存放该强区块引用的弱区块         
	//left等都是区块
	struct hashes *left, *right;
	struct hashes *parent;
	struct hashes *child;
	struct hashes *sibling;
}block;
//在block类中定义left，right等block结构体指针的目的是为了使区块们之间形成一定的逻辑结构（目前初步判断这个结构是类似于二叉排序树的结构）











//定义不完全区块
typedef struct incomplete_blocks{
	block *b;
	struct incomplete_blocks *next;
	unsigned long last_asked;
	uint32_t no_asks;
}block_incomplete;    //incomplete_blocks中定义了两个指针，第一个*b指向一个区块block，第二个
                      //*next指向一个本身incomplete_blocks，之后定义了两个属性last_asked和no_asks





//定义区块链类
class Blockchain
{
public:

	Blockchain(BlockHash hashes[]);//Blockchain类的构造方法，构造一个区块链
	//接收块
	bool add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, bool &added );
	void specific_print_blockchain();
	block *find_block_by_hash_and_chain_id( BlockHash hash, uint32_t chain_id );
	block *find_incomplete_block_by_hash_and_chain_id(BlockHash hash, uint32_t chain_id);
	block_incomplete *get_incomplete_chain( uint32_t chain_id);
	block *get_deepest_child_by_chain_id( uint32_t chain_id) ;
	bool have_full_block( uint32_t chain_id, BlockHash hash);
    bool still_waiting_for_full_block(  BlockHash hash, unsigned long time_of_now);
	bool add_block_by_parent_hash_and_chain_id( BlockHash parenthash, BlockHash new_block, uint32_t chain_id, network_block nb);
	vector<BlockHash> get_incomplete_chain_hashes( uint32_t chain_id , unsigned long time_of_now );
	void remove_waiting_blocks( unsigned long time_of_now );
	vector< pair <BlockHash, uint32_t> > get_non_full_blocks( unsigned long time_of_now );
	
	void set_block_full( uint32_t chain_id, BlockHash hash, string misc );
	void add_mined_block();

	void update_blocks_commited_time();

	

	bool locker_write = false;//全局标志位
	std::mutex lock;//全局互斥锁
	std::condition_variable can_write;//全局条件变量
	//上面三行都是线程阻塞互斥的操作

	/********************************/
	bool add_received_weak_block(uint32_t chain_id, BlockHash hash, network_block nb, bool &added);	//添加弱块
	bool add_weak_block(uint32_t chain_id, network_block nb, BlockHash hash);	//添加弱块
	void get_unsited_blocks(vector<BlockHash>& unsited, uint32_t chain_id, weakblock* root);	//查找对应链的所有未被引用弱块，存储在数组unsited中
	void add_mined_weak_block();
	weakblock* get_weak_block(uint32_t chain_id);	//获取对应chain_id的根弱块
	weakblock* find_weak_block_by_hash_and_chain_id(BlockHash hash, uint32_t chain_id);	//查找弱块
	//将弱块设置为被引用
	void set_weak_block_sited( vector<BlockHash> vec_weakblocks, uint32_t chain_id);
	//重定义add_received_block
	bool add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, vector<BlockHash> vec_weakblocks, bool &added );

	bool have_full_weak_block( uint32_t chain_id, BlockHash hash);
	/********************************/

private:
	/********************************/
	weakblock* weak_store[MAX_CHAINS];//这个weakblock数组暂时存放每一轮挖矿中产生的弱块
	unsigned long total_received_weak_blocks;	//收到的区块的总数
	unsigned long mined_weak_blocks;		//矿工挖出的弱块的数量

	weakblock* insert_weak_block_by_hash(weakblock *root, weakblock *b, BlockHash hash);	//	插入弱块
	weakblock* find_weak_block_by_hash(weakblock *b, BlockHash hash);	//	查找弱块

	int find_number_of_weak_nodes( weakblock *r);
	/********************************/

	block *chains[MAX_CHAINS];//这是指针数组，这个数组中的每一个元素都是block类型
	block_incomplete *inchains[MAX_CHAINS];//这是指针数组，这个数组中的每一个元素都是incomplete类型
	block *deepest[MAX_CHAINS];//这是指针数组，这个数组中的每一个元素都是block类型    指针数组是多个指针变量，以数组形式存在内存当中。
	//上述三个指针数组都是blockchain区块链类的对象属性


	//map里面套map，第一层的key是BlockHash，value不知道是啥意思
	map<BlockHash,pair <int,unsigned long> > received_non_full_blocks;
	map<BlockHash,unsigned long > waiting_for_full_blocks;             //定义了Blockchain对象的两个map类型属性
	unsigned long mined_blocks, processed_full_blocks, total_received_blocks;
	unsigned long long receiving_latency;	//收到强块的延迟
	unsigned long receving_total;	//收到的强块的总数
	unsigned long receving_weak_total;	//收到的弱块的总数（验证通过的）
	unsigned long long commited_latency[NO_T_DISCARDS];
	unsigned long commited_total[NO_T_DISCARDS];
	unsigned long long partially_latency[NO_T_DISCARDS];
	unsigned long partially_total[NO_T_DISCARDS];

	block_incomplete *is_incomplete_hash( block_incomplete *l, BlockHash hash);
	block_incomplete *remove_one_chain( block_incomplete *l, block_incomplete *to_be_removed);

	void add_subtree_to_received_non_full( block *b, uint32_t chain_id );

	void print_blocks( block *root );
	block *find_block_by_hash( block *b, BlockHash hash );
	bool add_block_by_parent_hash( block **root, BlockHash parent, BlockHash hash);
	block *find_max_depth( block *r );

	block *find_deepest_child( block *r);
	int find_number_of_nodes( block *r);

	void print_one( block *r);
	void print_heaviest_chain ( block *r);
	void print_full_tree( block *root );

	block *insert_block_only_by_hash( block *r, BlockHash hash, block **newnode);
	void print_hash_tree( block *root);
	block *insert_one_node( block *r, block *subtree);
	block *insert_subtree_by_hash( block *r, block *subtree);

	block_incomplete *add_block_to_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash);
	bool is_in_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash);
	block *find_incomplete_block(block_incomplete *l, BlockHash child_hash);
	void print_all_incomplete_chains( block_incomplete *l);
	int find_number_of_incomplete_blocks(block_incomplete *l);

};




























#endif