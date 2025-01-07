
#include <stdint.h>
#include <string>
#include <iostream>
#include <sys/timeb.h>
#include "./Blockchain.hpp"
//#include "../network/MyServer.hpp"


using namespace std;


extern unsigned long time_of_start;


/*
 *
 * Blockchain 
 * 
 */


//返回一个区块（初始一个区块，初始一条链）
block *bootstrap_chain( BlockHash initial_hash)
{
	block *root = (block *)malloc(sizeof(block));
	root->is_full_block = false;
	root->hash = initial_hash;
	root->left = root->right = root->child = root->parent = root->sibling = NULL;//初始化区块的时候这一串都为空
	root->nb = NULL;//区块的internet部分也为空

	return root;
}

//通过hash在blockchain中递归寻找指定区块
block *Blockchain::find_block_by_hash( block *b, BlockHash hash )//Blockchain区块链类对象的方法，返回block
{

	if (NULL == b) return NULL;

	
	if (b->hash > hash ) return find_block_by_hash( b->left, hash );
	else if (b->hash < hash ) return find_block_by_hash( b->right, hash);

	return b;

} //貌似是通过比较hash值进行二叉排序树递归搜索，以区块b为起点，以传入的hash值为指标进行二叉排序树递归搜索，找到后返回hash值对应的block，没找到返回null


weakblock *Blockchain::find_weak_block_by_hash_and_chain_id( BlockHash hash, uint32_t chain_id )
{
   return find_weak_block_by_hash( this->weak_store[chain_id], hash );

}

weakblock* Blockchain::find_weak_block_by_hash(weakblock *b, BlockHash hash){
	if (NULL == b) return NULL;
	if (b->hash > hash ) return find_weak_block_by_hash( b->left, hash );
	else if (b->hash < hash ) return find_weak_block_by_hash( b->right, hash);
	else return b;
}

//同步本节点的弱块的引用位
void Blockchain::set_weak_block_sited( vector<BlockHash> vec_weakblocks, uint32_t chain_id){
	//首先对搜索vec_weakblocks中的区块是否在本节点中，如果在本节点中，将其引用位置为true
	if(vec_weakblocks.size() == 0) return;
	for(int i = 0; i < vec_weakblocks.size(); i++){
		weakblock *curBlock = find_weak_block_by_hash(weak_store[chain_id], vec_weakblocks[i]);
		if(curBlock != NULL && !curBlock->nb->besited){
			curBlock->nb->besited = true;
		}
	}
}



block *Blockchain::insert_block_only_by_hash( block *r, BlockHash hash, block **newnode)
{
	if (NULL == r){
		block *t = (block *)malloc(sizeof(block));
		t->hash = hash;
		t->is_full_block = false;
		t->left = t->right = t->child = t->sibling = t->parent = NULL;
		*newnode = t;
		return t;
	}


	if (r->hash >= hash ) 		r->left 	= insert_block_only_by_hash ( r->left,  hash, newnode );
	else if (r->hash < hash) 	r->right 	= insert_block_only_by_hash ( r->right, hash, newnode );

	return r;
}
//传入的三个参数，r是遍历起点，hash是插入的块的hash，**newnode是新建的node节点的地址，根据r的hash与传入hash值的比较，递归插入，构建排序二叉树

//？
block *Blockchain::insert_one_node( block *r, block *subtree)//在Blockchain中，以r为起点，根据subtree的hash值进行block插入，类似二叉排序树的插入过程
{
	if( NULL == r) return subtree;
	if( NULL == subtree ) return r;


	//如果r和subtree都不为null，则根据r和subtree的hash值比较，当r的left或right为null时，将subtree插入r->left或r->right，当r的left或right不为null时，往下进行递归。
	if ( r->hash > subtree->hash){
		if( r->left == NULL){
			r->left = subtree;
		}
		else
			r->left = insert_one_node(r->left, subtree);
	}
	else if ( r->hash < subtree->hash){
		if( r->right == NULL){
			r->right = subtree;
		}
		else
			r->right = insert_one_node(r->right, subtree);
	}
	else{
		cout <<"Same hash...bad"<<endl;
	}

	return r;


}

block *Blockchain:: insert_subtree_by_hash( block *r, block *subtree)//subtree在英文中是“子树”的意思
{
	if (NULL == subtree) return r;

	//subtree不为null
	block *left = subtree->left;
	block *right = subtree->right;

	subtree->left = subtree->right = NULL;
	r = insert_one_node( r, subtree );

	r = insert_subtree_by_hash(r, left);
	r = insert_subtree_by_hash(r, right);

	return r;
}//整个过程是将subtree自上而下进行节点拆分，然后自上而下将subtree中的区块按方法insert_one_node插入


void Blockchain::print_blocks( block *root )
{
	if( NULL == root) return;

	print_blocks(root->left);
	printf("%8lx : %4d : %8lx : %d %d %d %d %d :  %d \n", root->hash, root->nb->depth, (root->parent == NULL)?0:root->parent->hash, 
											root->left != NULL, root->right != NULL, root->child != NULL, root->parent != NULL , root->sibling!=NULL, root->nb->depth );
	print_blocks(root->right);
}

void Blockchain::print_full_tree( block *root )
{
	if( NULL == root ) return;

	//hex输出十六进制，dec输出10进制
	cout << hex<<root->hash << dec << "("<<root->nb->depth<<")"<< "  :  ";
	block *t = root->child;
	while (NULL != t){
		cout << hex<<t->hash << " ";
		t= t->sibling;
	}
	cout << endl;

	t = root->child;
	while (NULL != t){
		if (t->child != NULL)
			print_full_tree( t );
		t= t->sibling;
	}

}


bool Blockchain::add_block_by_parent_hash( block **root, BlockHash parent, BlockHash hash)
{
	// Find the parent block node by parent's blockhash
	block *p = find_block_by_hash( *root, parent );
	if (NULL == p){
		cout << "Cannot find parent for " << parent << endl;
		return false;
	}


	// Insert the new node (of the child)
	block *newnode = NULL;
	*root = insert_block_only_by_hash( *root, hash, &newnode );
	if( NULL == newnode ){
		cout << "Something is wrong, new node is NULL in 'add_child' " << endl;
		return false;
	}

	// Set the parent of the new node
	newnode->parent = p;

	// Set the new node as one of parents children
	if ( NULL == p->child ){
		p->child = newnode;
	}
	else{
		block *z = p->child;
		while ( z->sibling != NULL) z = z->sibling;
		z->sibling = newnode;
	}

	return true;
}



//这应该是找到所有区块的数量
int Blockchain::find_number_of_nodes( block *r)
{
	if( NULL == r) return 0;

	int n = 0;
	block *c = r->child;
	while( NULL != c ){
		n += find_number_of_nodes( c );
		c = c->sibling;
	}

	return 1 + n;
}//返回r节点的所有后代节点的数量

int Blockchain::find_number_of_weak_nodes( weakblock *r){
	if( NULL == r) return 0;

	int leftNum = find_number_of_weak_nodes(r->left);
	int rightNum = find_number_of_weak_nodes(r->right);

	return leftNum + rightNum + 1;
}

void Blockchain::print_one( block *r)
{
	if( NULL == r) return;

	print_one(r->parent);
	printf("%lx (%d) ", r->hash, (NULL != r->nb)?r->nb->depth:-1);
	printf("%lx ", r->hash);
}



/*
 *
 * Incomplete 
 *
 */


//该方法返回的是不完全区块
block_incomplete *Blockchain::remove_one_chain( block_incomplete *l, block_incomplete *to_be_removed)
{
	//以l为起点，通过next操作遍历，对to_be_removed进行删除
	if( NULL == l)
		return NULL;

	if( l == to_be_removed){
		block_incomplete *t = l->next;
		free(l);
		return t;	
	}
	else{
		block_incomplete *t = l;
		while( NULL != t && t->next != to_be_removed)
			t = t->next;

		if( t->next == to_be_removed){
			t->next = t->next->next;
			free(to_be_removed);
		}

		return l;
	}
}

//按hash值返回incomplete区块
block_incomplete *Blockchain::is_incomplete_hash( block_incomplete *l, BlockHash hash)//函数定义中的block_incomplete表示返回的是incomplete区块，*Blockchain表示该方法是在Blockchain的基础上操作的
{//该方法检查传入hash值对应的区块是不是incomplete区块
	if ( NULL == l) return NULL;

	block_incomplete *t = l;
	while( NULL != l && l->b->hash != hash )
		l = l->next;

	if (NULL != l && NULL != l->b && l->b->hash == hash)
		return l;

	return NULL;
}         //以l为起点，通过l->next遍历，找到符合输入hash值的incomplete区块，返回incomplete区块。

//判断符合传入参数的块是否存在
bool Blockchain::is_in_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash)
{

	if ( NULL == l) return false;

	block_incomplete *t = l;
	while( NULL != t){
		block *b = find_block_by_hash( t->b, child_hash );//要时刻注意incomplete区块的->b指向对应的完整block
		if (NULL != b && b->parent!= NULL && b->parent->hash == parent_hash)
			return true;
		t = t->next;//递归寻找符合传入的parent_hash和child_hash的incomplete区块
	}

	return false;
}

//寻找符合传入参数的incomplete块
block *Blockchain::find_incomplete_block(block_incomplete *l, BlockHash child_hash)
{

	if ( NULL == l) return NULL;

	block_incomplete *t = l;
	while( NULL != t){
		block *b = find_block_by_hash( t->b, child_hash );
		if (NULL != b )
			return b;
		t = t->next;
	}

	return NULL;
}

//？看不懂
block_incomplete *Blockchain::add_block_to_incomplete(block_incomplete *l, BlockHash parent_hash, BlockHash child_hash)
{

	if ( NULL == l){

		block *bl = NULL;
		bl = bootstrap_chain ( parent_hash  );
		add_block_by_parent_hash( &bl, parent_hash, child_hash ) ;

		block_incomplete *bi;
		bi = (block_incomplete *)malloc(sizeof(block_incomplete));
		bi->b = bl;
		bi->next = NULL;
		bi->last_asked = 0;
		bi->no_asks = 0;
		return bi;
	}

	block_incomplete *tmp = l, *penultimate;
	block_incomplete *ch = NULL, *ph = NULL;
	while( NULL != tmp  ){

		if ( NULL == ch ) ch = (find_block_by_hash( tmp->b, child_hash ) != NULL) ? tmp : NULL;
		if ( NULL == ph ) ph = (find_block_by_hash( tmp->b, parent_hash) != NULL) ? tmp : NULL;

		penultimate = tmp;
		tmp = tmp->next;
	}//在incomplete区块关系链上往后一直遍历，直到tmp=null
	
	// Neither parent nor child hash has been found
	if ( NULL == ch && NULL == ph ){
		block *bl = NULL;
		bl = bootstrap_chain ( parent_hash  );
		add_block_by_parent_hash( &bl, parent_hash, child_hash ) ;

		block_incomplete *bi;
		bi = (block_incomplete *)malloc(sizeof(block_incomplete));
		bi->b = bl;
		bi->next = NULL;
		bi->last_asked = 0;
		bi->no_asks = 0;
		penultimate->next = bi;
	}
	else if ( NULL == ch ){

		add_block_by_parent_hash( &(ph->b), parent_hash, child_hash ) ;
	}
	else if ( NULL == ph){
	
		block *bl = bootstrap_chain( parent_hash );
		block *tmp = ch->b;
	    bl = insert_subtree_by_hash( bl, ch->b );
	    bl->child = tmp;
	    tmp->parent = bl;
	    ch->b = bl;
	    ch->last_asked = 0;
	    ch->no_asks = 0;
	}
	else{
	
		block *Ztmp = ch->b;
		ph->b = insert_subtree_by_hash( ph->b, ch->b );
		block *parent_block = find_block_by_hash( ph->b, parent_hash);
		Ztmp->parent = parent_block;
		block *tmp = parent_block->child;
		if (NULL == tmp)
			parent_block->child = Ztmp;
		else{
			while(tmp->sibling != NULL)
				tmp = tmp->sibling;
			tmp->sibling = Ztmp;
		}


		l = remove_one_chain( l, ch );

	}


	return l;
}

void Blockchain::print_all_incomplete_chains( block_incomplete *l)
{
	if ( NULL == l) return;


	printf("\n");
	print_full_tree( l->b );
	print_all_incomplete_chains( l->next );

}


int Blockchain::find_number_of_incomplete_blocks(block_incomplete *l)
{
	if( NULL == l) return 0;

	int no = 0;
	while( NULL != l){
		no += find_number_of_nodes( l->b);
		l = l->next;
	}

	return no;
}





/*
 *
 * Blockchain
 *
 */

//初始化区块链
Blockchain::Blockchain(BlockHash hashes[MAX_CHAINS])
{
	cout <<"[ ] Bootstraping the blockchain ... ";//“初始化区块链”

timeb t;

	for(int i=0; i<CHAINS; i++){//CHAINS在configuration文件中定义



//printf("time:");
//ftime(&t);
//cout << t.time * 1000 + t.millitm << endl;

		this->chains[i] = bootstrap_chain( hashes[ i ] );//遍历每一个链，初始化每一个链的第一个区块   chains[i]代表的不是区块，而是一个链
		this->inchains[i] = NULL;//inchains属性是block_incomplete属性
		//chains[i]是存储完整区块，inchains[i]是存储incomplete区块

		this->chains[i]->is_full_block = true;//chains[i]区块已经初始化完毕，is_full_block属性设置为true


                printf("=============== [member:]%d",i);
                printf("time:");
                ftime(&t);
                cout << t.time * 1000 + t.millitm << endl;

		this->chains[i]->nb = new(network_block);//初始化该区块（chains[i]）的network_block部分（也就是nb）
		this->chains[i]->nb->depth  = 0;
		this->chains[i]->nb->rank = 0;
		this->chains[i]->nb->next_rank = 0;
		this->chains[i]->nb->time_mined = 0;
		this->chains[i]->nb->time_received = 1;//上面几行设置了该区块的nb的一些属性
		for( int j=0; j<NO_T_DISCARDS; j++){
			this->chains[i]->nb->time_commited[j] = 1;
			this->chains[i]->nb->time_partial[j]= 1;//将该块的time_commited和time_partial数组内容都设置为1
		}
						

		this->deepest[i] = this->chains[i];//deepest[i]表示i链的最深区块，现在将i链刚刚初始化的区块chains[i]赋值给deepest[i]
		this->weak_store[i] = NULL;
	}

	received_non_full_blocks.clear();
	waiting_for_full_blocks.clear();//清空Blockchain对象的两个map中的内容
	processed_full_blocks = 0;
	total_received_blocks = 0;
	total_received_weak_blocks = 0;
	mined_blocks = 0;
	mined_weak_blocks = 0;

	


	receiving_latency = receving_total = 0;
	for( int j=0; j< NO_T_DISCARDS; j++){
		commited_latency[j] = 0;
		commited_total[j] = 0;
		partially_latency[j] = 0;
		partially_total[j] = 0;
	}

	//以上操作将Blockchain对象的一些属性初始化

	cout <<" Done !" << endl;
	fflush(stdout);//在使用多个输出函数连续进行多次输出时，有可能发现输出错误。因为下一个数据在上一个数据还没输出完毕，
	               //还在输出韩冲去时，下一个printf就把另一个数据加入输出缓冲区，结果冲掉了原来的数据，出现输出错误，fflush(stdout)强制马上输出，避免错误。
}


//打印整个区块链的hash树
void Blockchain::print_hash_tree( block *root)
{
	if( NULL == root) return;

	print_hash_tree(root->left);
	printf("Z: %lx\n", root->hash);
	print_hash_tree(root->right);


}//先序遍历二叉树

void Blockchain::add_subtree_to_received_non_full( block *b, uint32_t chain_id )//该方法的名字翻译过来是向received_non_full_blocks加入subtree节点
{																				//chain_id代表传入区块b所在的chain
	if( NULL == b) return;

	//b不等于null
	BlockHash hash = b->hash;
	if ( received_non_full_blocks.find(hash) == received_non_full_blocks.end() && !have_full_block(chain_id, hash) ){
		//ifreceived_non_full_blocks中对应的hash值是最后一个元素且在chain_id链中该hash对应的区块不是完整区块
		received_non_full_blocks.insert( make_pair( hash, make_pair( chain_id, 0) ) );
		total_received_blocks ++;
	}

	block *c = b->child;
	while( NULL != c ){
		add_subtree_to_received_non_full( c, chain_id );
		c = c->sibling;//sibling在英文中的意思是“兄弟姐妹”
	}
	//将b的不完整的孩子节点区块都插入到received_non_full_blocks中
}

//找到最深的块，但是最深是什么意思？
block *Blockchain::find_max_depth( block *r )
{
	if( NULL == r) return NULL;

	block *mx = r;
	block *tmp = r->child;
	while( NULL != tmp ){
		block *fm = find_max_depth( tmp );
		if( NULL != fm && fm->nb->depth > mx->nb->depth )
			mx = fm;
		tmp = tmp->sibling;
	}

	return mx;//返回的mx块的mx->nb->depth是最大的

}
//以r为起点，遍历，获得最深depth



/*
 * Add the block to the main/incomplete chain
 * Return true if the parent hash is not in any of the main/incomplete chains
 */

//A::B表示B是A类在hpp文件中定义的方法
/*
bool Blockchain::add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, bool &added )
{

	added = false;


	// If block is already in the chain, then do nothing
	if ( find_block_by_hash( this->chains[chain_id], hash ) != NULL ) return false;//发现要添加块的hash已经在Blockchain的chain_id链中，则放弃添加

	added = true;//发现没有要添加块的hash，开始添加操作


	// If parent hash is already in the tree, then just add the child
	if ( find_block_by_hash( this->chains[chain_id], parent ) != NULL ){


		// Check if child hash is in incompletes
		block_incomplete *bi= this->is_incomplete_hash( this->inchains[chain_id], hash);//如果is_incomplete_hash函数返回null，则
		                                                                                //表明该hash对应的区块不是incomplete区块，否则，就是incomplete区块
		//当传入hash对应的区块是incomplete区块时，将该区块赋值给bi
		if (bi != NULL){


			block *parent_block = find_block_by_hash( this->chains[chain_id], parent);//找到parent区块，赋值给parent_block
			block *child_block = bi->b;//将incomplete区块中的b（*b指向一个完整区块）赋值给child_block
			this->chains[chain_id] = insert_subtree_by_hash( this->chains[chain_id], child_block );//将child_block子树插入chains[chain_id]节点

			add_subtree_to_received_non_full( child_block, chain_id  );//将child_block节点插入received_non_full


			child_block->parent = parent_block;
			block *tmp = parent_block->child;
			if (NULL == tmp)
				parent_block->child = child_block;
			else{
				while(tmp->sibling != NULL)
					tmp = tmp->sibling;
				tmp->sibling = child_block;
			}
			//上面这一系列操作是将child_block的parent设置为parent_block,将parent_block的child设置为child_block（如果parent_block的child不为空的话往下遍历，直到为空的时候插入）

			this->inchains[chain_id] = this->remove_one_chain( this->inchains[chain_id], bi );//将不完整区块指针数组inchains中的bi给删除
			//根据之前的代码和这行代码，我猜测每次传入两个区块参数的方法中第一个区块参数是一个标志位，通过这个标识位再结合第二个区块参数进行相应操作

		}
		else{//当传入hash对应的区块不是incomplete区块时
			// Just add the (parent, hash)
			add_block_by_parent_hash( &(this->chains[chain_id]), parent, hash);

			// Add to the non-full-blocks
			if ( received_non_full_blocks.find(hash) == received_non_full_blocks.end() && !have_full_block(chain_id, hash) ){
				received_non_full_blocks.insert( make_pair( hash, make_pair( chain_id, 0) ) );
				total_received_blocks ++;
			}

		}


		// Add full block info
		block *bz = find_block_by_hash( this->chains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
			added = true;

			//  Update deepest
			uint32_t old_depth = deepest[chain_id]->nb->depth;
			block *deep_last = find_max_depth( bz );
			if( deep_last->nb->depth > old_depth)
				deepest[ chain_id] = deep_last;//将[ chain_id]更新

		}

	}

	// parent节点不在chains[chain_id]树中
	// Else, need to add to incomplete chain and ask for more 
	else{

		if ( is_in_incomplete( this->inchains[chain_id], parent, hash) ) {
			//以this->inchains[chain_id]
			added = false;
			return false;//如果传入参数的incomplete节点在this->inchains[chain_id]中，则不插入，以防止重复插入
		}

		//此时传入参数的incomplete节点不在this->inchains[chain_id]中，需要将该节点插入this->inchains[chain_id]中
		// Add this to incomplete chain
    	this->inchains[chain_id] = add_block_to_incomplete( this->inchains[chain_id], parent, hash );


    	block *bz = find_incomplete_block( this->inchains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
		}



		// Ask for parent hash
		return true;

	}

	return false;
	
}
*/

bool Blockchain::add_received_block( uint32_t chain_id, BlockHash parent, BlockHash hash, network_block nb, vector<BlockHash> vec_weakblocks, bool &added )
{
	added = false;

	// If block is already in the chain, then do nothing
	if ( find_block_by_hash( this->chains[chain_id], hash ) != NULL ) return false;//发现要添加块的hash已经在Blockchain的chain_id链中，则放弃添加

	added = true;//发现没有要添加块的hash，开始添加操作
	

	// If parent hash is already in the tree, then just add the child
	if ( find_block_by_hash( this->chains[chain_id], parent ) != NULL ){

		// 先检查该块是否在inchains上
		// Check if child hash is in incompletes
		block_incomplete *bi= this->is_incomplete_hash( this->inchains[chain_id], hash);
		                                                                            
		//如果在inchains上
		if (bi != NULL){

			//在链上找该不完全块的父块
			block *parent_block = find_block_by_hash( this->chains[chain_id], parent);//找到parent区块，赋值给parent_block
			//不完全块的中的块就是该父块的孩子
			block *child_block = bi->b;//将incomplete区块中的b（*b指向一个完整区块）赋值给child_block
			//然后将该块child_block插入到对应的链上
			this->chains[chain_id] = insert_subtree_by_hash( this->chains[chain_id], child_block );//将child_block子树插入chains[chain_id]节点

			//将该孩子块的父块设置为父块
			child_block->parent = parent_block;
			
			block *tmp = parent_block->child;
			if (NULL == tmp)
				parent_block->child = child_block;//如果父块的孩子为空，则将该块设置为父块的孩子
			else{
				while(tmp->sibling != NULL)
					tmp = tmp->sibling;
				tmp->sibling = child_block;	//否则将该块设置为父块的sibling（兄弟姐妹）
			}
			//上面这一系列操作是将child_block的parent设置为parent_block,将parent_block的child设置为child_block（如果parent_block的child不为空的话往下遍历，直到为空的时候插入）
			//然后将对应的不完全块从不完全链上移除
			this->inchains[chain_id] = this->remove_one_chain( this->inchains[chain_id], bi );//将不完整区块指针数组inchains中的bi给删除

		}
		else{
			//当传入hash对应的区块不是incomplete区块时
			// Just add the (parent, hash)
			add_block_by_parent_hash( &(this->chains[chain_id]), parent, hash);
		}

		// Add full block info
		// 对已经在链上的块的信息进行更新，然后更新链的最深长度和最深块
		block *bz = find_block_by_hash( this->chains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
			//更新区块的时间信息
			receving_total++;
		    unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
		    if( time_of_now > bz->nb->time_mined ){ 
		    	bz->nb->time_received = time_of_now;
				//下面应该统计总共收到的弱块的数量
			    receiving_latency += bz->nb->time_received - bz->nb->time_mined;
		    }
		    else bz->nb->time_received = bz->nb->time_mined;

			//添加引用的弱块的数组
			bz->number_of_weakblocks = vec_weakblocks.size();
			bz->vec_weakblocks = (BlockHash*)malloc(sizeof(BlockHash) * bz->number_of_weakblocks);
			for(int i = 0; i < bz->number_of_weakblocks; i++){
				bz->vec_weakblocks[i] = vec_weakblocks[i];
			}
			
			added = true;

			//  Update deepest
			uint32_t old_depth = deepest[chain_id]->nb->depth;
			block *deep_last = find_max_depth( bz );
			if( deep_last->nb->depth > old_depth)
				deepest[ chain_id] = deep_last;//将[ chain_id]更新

		}

	}

	// parent节点不在chains[chain_id]树中
	// Else, need to add to incomplete chain and ask for more 
	else{
		
		if ( is_in_incomplete( this->inchains[chain_id], parent, hash) ) {
			//以this->inchains[chain_id]
			added = false;
			return false;//如果传入参数的incomplete节点在this->inchains[chain_id]中，则不插入，以防止重复插入
		}

		//此时传入参数的incomplete节点不在this->inchains[chain_id]中，需要将该节点插入this->inchains[chain_id]中
		// Add this to incomplete chain
    	this->inchains[chain_id] = add_block_to_incomplete( this->inchains[chain_id], parent, hash );


    	block *bz = find_incomplete_block( this->inchains[chain_id], hash );
		if( NULL != bz ){
			bz->nb = new network_block(nb);
			receving_total++;
		    unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
		    if( time_of_now > bz->nb->time_mined ){ 	
		    	bz->nb->time_received = time_of_now;
				//下面应该统计总共收到的弱块的数量
			    receiving_latency += bz->nb->time_received - bz->nb->time_mined;
		    }
		    else bz->nb->time_received = bz->nb->time_mined;
			//添加引用的弱块的数组
			bz->number_of_weakblocks = vec_weakblocks.size();
			bz->vec_weakblocks = (BlockHash*)malloc(sizeof(BlockHash) * bz->number_of_weakblocks);
			for(int i = 0; i < bz->number_of_weakblocks; i++){
				bz->vec_weakblocks[i] = vec_weakblocks[i];
			}
		}



		// Ask for parent hash
		return true;

	}

	return false;
	
}

bool Blockchain::add_received_weak_block(uint32_t chain_id, BlockHash hash, network_block nb, bool &added){
	added = false;
	if ( find_weak_block_by_hash( this->weak_store[chain_id], hash ) != NULL ) return false;

	added = true;//发现没有要添加块的hash，开始添加操作
	weakblock *wb = (weakblock *)malloc(sizeof(weakblock));
	wb->hash = hash;
	wb->nb = new network_block(nb);
	wb->is_full_block = false;
	//wb->left = wb->right = wb->child = wb->sibling = wb->parent = NULL;
	wb->left = wb->right = NULL;
	this->weak_store[chain_id] = insert_weak_block_by_hash(this->weak_store[chain_id], wb, hash);
	//将收到的弱区块插入received_non_full_blocks
	
	if ( received_non_full_blocks.find(hash) == received_non_full_blocks.end() && !have_full_weak_block(chain_id, hash) ){
			received_non_full_blocks.insert( make_pair( hash, make_pair( chain_id, 0) ) );
			//total_received_blocks ++;	
			total_received_weak_blocks ++;	//收到的弱块的总数
	}
	return true;
}

weakblock* Blockchain::insert_weak_block_by_hash(weakblock *root, weakblock *b, BlockHash hash){
	if (root == NULL){
		return b;
	}

	if (root->hash >= hash ) 		root->left 	= insert_weak_block_by_hash ( root->left,  b, hash );
	else if (root->hash < hash) 	root->right = insert_weak_block_by_hash ( root->right, b, hash );

	return root;

}

//将本地矿工挖到的弱块加入弱块的存储结构中
bool Blockchain::add_weak_block(uint32_t chain_id, network_block nb, BlockHash hash){
	weakblock *wb = (weakblock *)malloc(sizeof(weakblock));
	wb->hash = hash;
	wb->nb = new network_block(nb);
	wb->is_full_block = false;
	wb->left = wb->right = NULL;
	this->weak_store[chain_id] = insert_weak_block_by_hash(this->weak_store[chain_id], wb, hash);
	return true;
}

//查找对应链的所有未被引用弱块，存储在数组unsited中
void Blockchain::get_unsited_blocks(vector<BlockHash>& unsited, uint32_t chain_id, weakblock* root){
	if(root == NULL){
		return;
	}
	if(!root->nb->besited && root->is_full_block){
		unsited.push_back(root->nb->hash);
		root->nb->besited = true;
	}
	if(root->left) get_unsited_blocks(unsited, chain_id, root->left);
	if(root->right) get_unsited_blocks(unsited, chain_id, root->right);
}	

//打印参数指标信息
void Blockchain::specific_print_blockchain()
{

    uint32_t tot_max_depth = 0, tot_no_nodes = 0, tot_no_in_nodes = 0;
	uint32_t tot_weak_no_nodes = 0;
    for( int i=0; i<CHAINS; i++){
        tot_max_depth += get_deepest_child_by_chain_id(i)->nb->depth;//将该Blockchain区块链的所有链的最深长度相加
        tot_no_nodes += find_number_of_nodes(  this->chains[i]);//计算该Blockchain区块链的所有完整节点
        tot_no_in_nodes += find_number_of_incomplete_blocks( this->inchains[i]); //计算该Blockchain区块链的所有incomplete节点
		tot_weak_no_nodes += find_number_of_weak_nodes( this->weak_store[i]);//计算该Blockchain区块链的所有weak_block节点

    }
    unsigned long time_of_finish = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
    float secs = (time_of_finish - time_of_start)/ 1000.0;

    tot_no_nodes -= CHAINS;


	printf("\n=============== [BLOCK HEADERS:]   Main tree:  %7d    Side trees: %7d  Total:  %7d\n", tot_no_nodes, tot_no_in_nodes, tot_no_nodes + tot_no_in_nodes );
	printf("\n=============== [MINING RATE  :]   %4d blocks / %.1f secs = %.2f  bps \n", tot_no_nodes+tot_no_in_nodes+tot_weak_no_nodes , secs, (float)(tot_no_nodes+tot_no_in_nodes+tot_weak_no_nodes)/secs );

	printf("\n=============== [BLOCKCHAIN   :]  Lengths:%7d   Mined:%5ld  Usefull:  %1.f %%  \n", 
												tot_max_depth, mined_blocks,   
												100.0*tot_max_depth / (tot_no_nodes+tot_no_in_nodes) );

	printf("\n=============== [BLOCKS       :]  Receving(%lu)  : %0.2f secs     Partially ", receving_total, receiving_latency/1000.0/receving_total );
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("(%lu) ", partially_total[j]);
	printf(": ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("%0.1f secs ", partially_latency[j]/1000.0/partially_total[j]);
	printf("   Committing ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("(%lu) ", commited_total[j]);
	printf(": ");
	for(int j=0; j<NO_T_DISCARDS; j++)
		printf("%0.1f secs ", commited_latency[j]/1000.0/commited_total[j]);
	printf(" \n");



	printf("\n=============== [WEAK BLOCKS  :]  Received:  %7ld / %7ld   Waiting:  %7ld   Processed:  %7ld ( %.0f %% ) \n", 
						total_received_weak_blocks, received_non_full_blocks.size(), 
						waiting_for_full_blocks.size(), 
						processed_full_blocks, (total_received_weak_blocks> 0) ? (100.0*processed_full_blocks/total_received_weak_blocks): 0 );




    fflush(stdout);



}



/*
*
*
*	NEW
*
*/

block *Blockchain::find_block_by_hash_and_chain_id( BlockHash hash, uint32_t chain_id )
{
   return find_block_by_hash( this->chains[chain_id], hash );

}

block *Blockchain::find_incomplete_block_by_hash_and_chain_id(BlockHash hash, uint32_t chain_id)
{
	return find_incomplete_block( this->inchains[chain_id], hash );
}



block_incomplete *Blockchain::get_incomplete_chain( uint32_t chain_id)//根据链id获得incomplete区块链
{
	return this->inchains[chain_id];
}



block *Blockchain::get_deepest_child_by_chain_id( uint32_t chain_id) 
{
	if( NULL == deepest[ chain_id] ){
		printf("Something is wrong with get_deepest_child_by_chain_id\n"); fflush(stdout);
		printf("on [%d] it returns NULL\n", chain_id ); fflush(stdout);
		exit(5);
	}
	return deepest[ chain_id ];//deepest[ chain_id ]是chain_id链的最深区块
}

//判断该区块链的指定链id的链中有没有指定hash的完整区块
bool Blockchain::have_full_block( uint32_t chain_id, BlockHash hash)
{
	block *bz = find_block_by_hash( this->chains[chain_id], hash );
	if( NULL != bz && bz->is_full_block) return true;
	return false;

}

bool Blockchain::have_full_weak_block( uint32_t chain_id, BlockHash hash)
{
	weakblock *wb = find_weak_block_by_hash( this->weak_store[chain_id], hash );
	if( NULL != wb && wb->is_full_block) return true;
	return false;

}

//？
bool Blockchain::still_waiting_for_full_block(  BlockHash hash, unsigned long time_of_now)
{
	if ( waiting_for_full_blocks.find( hash ) == waiting_for_full_blocks.end() ){
		waiting_for_full_blocks.insert( make_pair( hash, time_of_now ));
		return true;
	}
	return false;

}//如果传入的hash值是waiting_for_full_blocks的最后一个的话（最后一个的意思是不是代表没有？）就插入该hash值


bool Blockchain::add_block_by_parent_hash_and_chain_id( BlockHash parent_hash, BlockHash new_block, uint32_t chain_id, network_block nb)//这个方法在miner.cpp中使用，将生成的新块插入指定链id的链
{
	add_block_by_parent_hash( &(this->chains[chain_id]), parent_hash, new_block);

	block *bz = find_block_by_hash( this->chains[chain_id], new_block );
	if( NULL != bz ){
		this->deepest[chain_id] = bz;
		bz->nb = new network_block(nb);
	}

}


vector<BlockHash> Blockchain::get_incomplete_chain_hashes( uint32_t chain_id , unsigned long time_of_now )
{
	vector <BlockHash> hashes;//创建一个名为hashes的hash数组
    block_incomplete *t = inchains[chain_id];//inchains[i]和chains[i]都是对应的那条链的标志块（标志块可能是开头块），然后通过Block中定义的left，right等链接到该链的所有块
    while( NULL != t){
    	block_incomplete *nextt = t->next;
    	if ( time_of_now - t->last_asked  > ASK_FOR_INCOMPLETE_INDIVIDUAL_MILLISECONDS ){
    		t->last_asked = time_of_now;	//最近被询问的时间
    		t->no_asks ++;	//被询问次数
    		if( t->no_asks > NO_ASKS_BEFORE_REMOVING)
    			this->inchains[chain_id] = this->remove_one_chain( this->inchains[chain_id], t );	//移除该块
    		else
    			hashes.push_back( t->b->hash);	//否则将该时刻的不完全块加入返回数组
      	}
	    t = nextt;
    }

    return hashes;
}//得到指定chain_id的incomplete链的不完整区块的hash 


//遍历vector数组里的东西，进行相关操作（可能是获得full_blocks）
vector< pair <BlockHash, uint32_t> > Blockchain::get_non_full_blocks( unsigned long time_of_now )
{
	vector< pair <BlockHash, uint32_t> > nfb;
	vector <BlockHash> to_remove;
	
	//遍历整个received_non_full_blocks
	for( auto it=received_non_full_blocks.begin(); it != received_non_full_blocks.end(); it++ )
		if( time_of_now - it->second.second > ASK_FOR_FULL_BLOCKS_INDIVIDUAL_EACH_MILLISECONDS){
			
			it->second = make_pair ( it->second.first, time_of_now ) ;
			//block *bz = find_block_by_hash( this->chains[it->second.first], it->first );
			
			weakblock* bz = find_weak_block_by_hash(this->weak_store[it->second.first], it->first);
			if( NULL != bz && ! (bz->is_full_block) )
				nfb.push_back( make_pair(it->first, it->second.first));
			else if ( NULL != bz && bz->is_full_block )
				to_remove.push_back( it->first );

			if( nfb.size() >= MAX_ASK_NON_FULL_IN_ONE_GO ) break;

		}


	for( int i=0; i<to_remove.size(); i++)
		if( received_non_full_blocks.find(  to_remove[i]) != received_non_full_blocks.end() )
			received_non_full_blocks.erase( to_remove[i] );
		
	return nfb;
}


//遍历vector数组里的东西，进行相关操作（可能是移除正在等待的区块）
void Blockchain::remove_waiting_blocks( unsigned long time_of_now )
{
	
	vector <BlockHash> to_remove;

	for( auto it=waiting_for_full_blocks.begin(); it != waiting_for_full_blocks.end(); it++ ){
		if( time_of_now - it->second > MAX_WAIT_FOR_FULL_BLOCK_MILLSECONDS)
			//waiting_for_full_blocks.erase( (it++)->first );
			to_remove.push_back( it->first );
	}//遍历waiting_for_full_blocks，将其中超时的block信息提取到to_remove中

	for( int i=0; i<to_remove.size(); i++)
		if( waiting_for_full_blocks.find(to_remove[i]) != waiting_for_full_blocks.end() )
			waiting_for_full_blocks.erase( to_remove[i] );//将waiting_for_full_blocks中超时的block剔除

}




void Blockchain::set_block_full( uint32_t chain_id, BlockHash hash, string misc )
{
	if( received_non_full_blocks.find(hash) != received_non_full_blocks.end() )//map中end指向最后一个元素的后一个元素，map.find!=map.end是判断find元素是否存在，因为find元素不存在的话返回的是end
		received_non_full_blocks.erase(hash);
	if( waiting_for_full_blocks.find(hash) != waiting_for_full_blocks.end() )
		waiting_for_full_blocks.erase(hash);
	//上面是将hash值对应的区块从received_non_full_blocks和waiting_for_full_blocks中删除

	//block *bz = find_block_by_hash( this->chains[chain_id], hash );//在对应的链中提取对应hash的区块赋值给bz
	weakblock *bz = find_weak_block_by_hash( this->weak_store[chain_id], hash );
	if (NULL != bz){
		bz->is_full_block = true;
		processed_full_blocks ++ ;

        // Define time_received
        if ( bz -> nb != NULL){
		    unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
		    if( time_of_now > bz->nb->time_mined ){ 	
		    	bz->nb->time_received = time_of_now;
				//下面应该统计总共收到的弱块的数量
			    //receving_total++;
				receving_weak_total++;
			    //receiving_latency += bz->nb->time_received - bz->nb->time_mined;
				//receiving_weak_latency += bz->nb->time_received - bz->nb->time_mined;
		    }
		    else 									
		    	bz->nb->time_received = bz->nb->time_mined;
			//上面几行代码是对区块的各种指标时间进行设定和记录，为了后面输出区块链的性能信息

		    /*
			if ( STORE_BLOCKS && (hash % BLOCKS_STORE_FREQUENCY) == 0  ){
				string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
	            ofstream file;
	            file.open(filename, std::ios_base::app); 
	            file << hex << hash << dec << " " << (bz->nb->time_received - bz->nb->time_mined) << " " << misc << endl;
	            file.close();
	        }
	        */
	        if ( STORE_BLOCKS && (hash % BLOCKS_STORE_FREQUENCY) == 0  ){
				string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
	            ofstream file;
	            file.open(filename, std::ios_base::app); //std::ios_base::app: 每次进行写入操作的时候都会重新定位到文件的末尾.
	            file << "0 " << hex << hash << dec << " " << (bz->nb->time_received - bz->nb->time_mined) << endl;
	            file.close();
	        }//上述代码是负责存储之类的操作

		}

	}
}

void Blockchain::add_mined_block()
{
	mined_blocks++;//Blockchain对象的属性，应该是记录挖了多少区块
}

void Blockchain::add_mined_weak_block()
{
	mined_weak_blocks++;
}

weakblock* Blockchain::get_weak_block(uint32_t chain_id){
	return this->weak_store[chain_id];
}

void Blockchain::update_blocks_commited_time()
{

	unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
	//std::chrono::system_clock::now().time_since_epoch()表示获得从1970年1月1日到现在的时间，std::chrono::milliseconds(1）是一毫秒，所以上面一行代码的意思就是获得从1970年到现在的时间（以毫秒为单位）

	for( int j=0; j<NO_T_DISCARDS;j ++){


			/*
			 * Update partial times
			 */
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t) continue;

				while ( NULL != t  ){
					if (  NULL != t->nb && 0 == t->nb->time_partial[j]   &&  time_of_now > t->nb->time_mined ){
						t->nb->time_partial[j] = time_of_now;
						partially_total[j] ++;
						partially_latency[j] += t->nb->time_partial[j] - t->nb->time_mined;

				        if ( STORE_BLOCKS && (t->hash % BLOCKS_STORE_FREQUENCY) == 0  ){
							string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
				            ofstream file;
				            file.open(filename, std::ios_base::app); 
				            file << "1 " << hex << t->hash << dec << " " << (t->nb->time_partial[j] - t->nb->time_mined) << " " << j << endl;
				            file.close();
				        }


					}
					t = t->parent;
				}
			}


			/*
			 * Full commit times
			 */


			// Find the minimal next_rank
			bool stop_this_j = false;
			uint32_t confirm_bar = -1;
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t){
					stop_this_j = true;
					break;
					//return;
				}
				
				if ( t->nb == NULL ){
					stop_this_j = true;
					break;
					//return;
				}

				if( stop_this_j ) break;

				if ( t->nb->next_rank < confirm_bar ) confirm_bar = t->nb->next_rank;
			}

			if( stop_this_j) continue;


			if ( confirm_bar < 0) continue;


			// Update commited times
			for( int i=0; i<CHAINS; i++){

				// Discard the last 
				block *t = deepest[i];
				int count = 0;
				while( NULL != t && count++ < T_DISCARD[j] )
					t = t->parent;
				if (NULL == t) continue;

				while ( NULL != t  ){
					if ( NULL != t->nb &&  0 == t->nb->time_commited[j]   &&  time_of_now > t->nb->time_mined ){
						t->nb->time_commited[j] = time_of_now;
						commited_total[j] ++;
						commited_latency[j] += t->nb->time_commited[j] - t->nb->time_mined;
						
						if ( STORE_BLOCKS && (t->hash % BLOCKS_STORE_FREQUENCY) == 0  ){
							string filename =  string(FOLDER_BLOCKS)+"/"+my_ip+"-"+to_string(my_port);
				            ofstream file;
				            file.open(filename, std::ios_base::app); 
				            file << "2 " << hex << t->hash << dec << " " << (t->nb->time_commited[j] - t->nb->time_mined) << " " << j <<  endl;
				            file.close();
				        }
					}
					t = t->parent;
				}

			}

	}

}
