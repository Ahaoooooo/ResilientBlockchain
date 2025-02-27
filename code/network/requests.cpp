

#include "./requests.hpp"
#include "../utils/misc.h"

extern string my_ip;
extern uint32_t my_port; 

    


//创建ask_block的报文
string create__ask_block( uint32_t chain_id, BlockHash hash, uint32_t my_depth, uint32_t hash_depth )
{
	  string s = "#ask_block,"+my_ip+","+to_string(my_port)+","+to_string(chain_id)+","+to_string(hash);
	  s += ","+ to_string( ( my_depth >= hash_depth)? 1:(hash_depth - my_depth) );
	  return s;
}

//处理ask_block的报文
bool parse__ask_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, uint32_t &max_number_of_blocks   )
{
	if ( sp.size() < 6 ) return false;
  //sp是把报文按‘,'切割之后的一个字符串数组；passed用来放置报文重复，如果收到了几个重复的报文，只处理收到的第一个报文，防止重复处理
  //如果该请求重复的话，就不解析；该请求不重复的话，就解析
	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1]; 
    sender_port = safe_stoi(sp[2], pr);
    chain_id = safe_stoi(sp[3], pr);
    hash = safe_stoull( sp[4], pr );
    max_number_of_blocks = safe_stoi( sp[5], pr);

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0  && chain_id < CHAINS ) ){
      cout << "Could not get proper values of ask_block"<<endl;
      cout << pr << " " << sender_ip << " " << chain_id << endl;
      return false;
    }

    return true;
}




//创建形式为string的process_block
string create__process_block( network_block *nb )
{
    string s = "#process_block,"+my_ip+","+to_string(my_port)+","+to_string(nb->chain_id)+","+to_string(nb->parent)+","+to_string(nb->hash)+",";
    s += to_string(nb->no_txs) + ",";
    s += to_string(nb->depth) + ",";
    s += to_string(nb->rank) + ",";
    s += to_string(nb->next_rank);

    return s;
}


bool parse__process_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, network_block &nb   )
{
    int MPL = merkle_proof_length();

    //如果该请求格式不对，则不解析
  	if ( sp.size() < 10 ) return false;
    //如果该请求重复，则不解析
  	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4]+sp[5], passed ) ) return false;

    //将传过来的报文解析为区块的具体值
    bool pr = true;
    sender_ip         = sp[1];
    sender_port       = safe_stoi(    sp[ 2], pr);
    nb.chain_id       = safe_stoi(    sp[ 3], pr);
    nb.parent         = safe_stoull(  sp[ 4], pr );
    nb.hash           = safe_stoull(  sp[ 5], pr );
    nb.no_txs         = safe_stoi(    sp[ 6], pr);
    nb.depth          = safe_stoi(    sp[ 7], pr );
    nb.rank           = safe_stoi(    sp[ 8], pr);
    nb.next_rank      = safe_stoi(    sp[ 9], pr );


    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && nb.chain_id < CHAINS ) ){
          cout << "Could not get proper values of process_block"<<endl;
          cout << pr << " " << sender_ip << " " << nb.chain_id << endl;

          for( int i=1; i<=9; i++)
            cout << sp[i] << endl;

          return false;
    }
    return true;

}


string create__process_weak_block( network_block *nb ){
  string s = "#process_weak_block,"+my_ip+","+to_string(my_port)+","+to_string(nb->chain_id)+","+to_string(nb->parent)+","+to_string(nb->hash)+",";
    s += to_string(nb->no_txs) + ",";
    s += to_string(nb->depth) + ",";
    s += to_string(nb->rank) + ",";
    s += to_string(nb->next_rank);

    return s;

}

bool parse__process_weak_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, network_block &nb){
    int MPL = merkle_proof_length();

    //如果该请求格式不对，则不解析
  	if ( sp.size() < 10 ) return false;
    //如果该请求重复，则不解析
  	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4]+sp[5], passed ) ) return false;

    //将传过来的报文解析为区块的具体值
    bool pr = true;
    sender_ip         = sp[1];
    sender_port       = safe_stoi(    sp[ 2], pr);
    nb.chain_id       = safe_stoi(    sp[ 3], pr);
    nb.parent         = safe_stoull(  sp[ 4], pr );
    nb.hash           = safe_stoull(  sp[ 5], pr );
    nb.no_txs         = safe_stoi(    sp[ 6], pr);
    nb.depth          = safe_stoi(    sp[ 7], pr );
    nb.rank           = safe_stoi(    sp[ 8], pr);
    nb.next_rank      = safe_stoi(    sp[ 9], pr );


    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && nb.chain_id < CHAINS ) ){
          cout << "Could not get proper values of process_weak_block"<<endl;
          cout << pr << " " << sender_ip << " " << nb.chain_id << endl;

          for( int i=1; i<=9; i++)
            cout << sp[i] << endl;

          return false;
    }
    return true;

}

string create__got_full_block(uint32_t chain_id, BlockHash hash )
{
	   string s = "#got_full_block,"+my_ip+","+to_string(my_port)+","+to_string(chain_id)+","+to_string(hash);
	   return s;
}

bool parse__got_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  )
{

	if ( sp.size() < 5 ) return false;
	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1]; 
    sender_port = safe_stoi(sp[2], pr);
    chain_id = safe_stoi(sp[3], pr);
    hash = safe_stoull( sp[4], pr );

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && chain_id < CHAINS ) ){
      cout << "Could not get proper values of got_full_block"<<endl;
      cout << pr << " " << sender_ip << " " << chain_id << endl;
      return false;
    }

    return true;
}



string create__have_full_block( uint32_t chain_id, BlockHash hash)
{
  		string s = "#have_full_block,"+my_ip+","+to_string(my_port)+","+to_string(chain_id)+","+to_string(hash);
  		return s;
}
bool parse__have_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  )
{

	if ( sp.size() < 5 ) return false;
	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1]; 
    sender_port = safe_stoi(sp[2], pr);
    chain_id = safe_stoi(sp[3], pr);
    hash = safe_stoull( sp[4], pr );

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && chain_id < CHAINS ) ){
      cout << "Could not get proper values of have_full_block"<<endl;
      cout << pr << " " << sender_ip << " " << chain_id << endl;
      return false;
    }

    return true;
}




string create__ask_full_block( uint32_t chain_id, BlockHash hash)
{
	  string s = "#ask_full_block,"+my_ip+","+to_string(my_port)+","+to_string(chain_id)+","+to_string(hash);
	  return s;
}
bool parse__ask_full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash  )
{

	if ( sp.size() < 5 ) return false;
	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1]; 
    sender_port = safe_stoi(sp[2], pr);
    chain_id = safe_stoi(sp[3], pr);
    hash = safe_stoull( sp[4], pr );

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! ( pr && sender_ip.size()> 0 && chain_id < CHAINS) ){
      cout << "Could not get proper values of ask_full_block"<<endl;
      cout << pr << " " << sender_ip << " " << chain_id << endl;
      return false;
    }

    return true;
}



string create__full_block( uint32_t chain_id, BlockHash hash, tcp_server *ser, Blockchain *bc )
{
    string s = "#full_block,"+my_ip+","+to_string(my_port)+","+to_string(chain_id)+","+to_string(hash)+",";

    //block * b = bc->find_block_by_hash_and_chain_id( hash, chain_id ); 
    weakblock* b = bc->find_weak_block_by_hash_and_chain_id( hash, chain_id);
    if( WRITE_BLOCKS_TO_HDD ){
        string filename = ser->get_server_folder()+"/"+blockhash_to_string( hash );
        ifstream file;
        try{ file.open(filename); }
        catch(const std::string& ex){  return ""; }

        stringstream buffer;
        buffer << file.rdbuf();
        s += buffer.str(); 

        if( buffer.str().size() < 10){
          cout<<"Providing:"<<s<<":"<<endl;
          cout <<"BAD buffer"<< endl;
          fflush(stdout);
          exit(1);
        }
    }
    else{
        string tx;
        if ( NULL != b && b->nb->no_txs > 0 ){
            for( int j=0; j< b->nb->no_txs; j++){
              tx = create_one_transaction();
              s += tx + "\n";
            }
        }
    }

    // Add everything removed from process_block 
    network_block *nb = b->nb;
    if (NULL != nb){
      s += ",";
      s += to_string(nb->trailing) + "," +to_string(nb->trailing_id) +","+ nb->merkle_root_chains + "," + nb->merkle_root_txs + ",";
      for(int i=0; i<nb->proof_new_chain.size(); i++) s += nb->proof_new_chain[i]+",";
      
      s += to_string(nb->time_mined) ;
      unsigned long time_of_now = std::chrono::system_clock::now().time_since_epoch() /  std::chrono::milliseconds(1);
      s += ","+to_string(time_of_now);

    }
    else{

      printf("Network block cannot be found in create__full_block");
      exit(2);
    }

    return s;
}


bool parse__full_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, uint32_t &chain_id, BlockHash &hash, string &txs, network_block &nb, unsigned long &sent_time  )
{
  int MPL = merkle_proof_length();


  if ( sp.size() < 10+1*MPL + 1 ) return false;
  if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1];
    sender_port = safe_stoi(sp[2], pr);
    chain_id = safe_stoi(sp[3], pr);
    hash = safe_stoull( sp[4], pr);
    txs = sp[5];

    nb.trailing             = safe_stoull( sp[6], pr );
    nb.trailing_id          = safe_stoull( sp[7], pr );
    nb.merkle_root_chains   = sp[8];
    nb.merkle_root_txs      = sp[9];
    for(int j=0; j<MPL; j++)  nb.proof_new_chain.push_back(sp[10+j]);
    nb.time_mined   = safe_stoull( sp[10+1*MPL], pr );
    sent_time   = safe_stoull( sp[10+1*MPL+1], pr );


    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && chain_id < CHAINS  ) ){
      cout << "Could not get proper values of full_block:"<<sp.size()<< endl;
      cout << pr << " " << sender_ip << " " << chain_id << endl;
      for( int i=0; i<sp.size(); i++)
        cout << sp[i]<<endl;
      return false;
    }

    return true;
}


string create__ping( string tt, uint32_t dnext, unsigned long tsec, int mode)
{
    string s = "#ping,"+my_ip+","+to_string(my_port)+","+tt+","+to_string(dnext)+","+to_string(tsec)+","+to_string(mode);
    return s;
}

bool parse__ping( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, string &tt, uint32_t &dnext, unsigned long &tsec , int &mode  )
{
  if ( sp.size() < 7 ) return false;
  if ( key_present( sp[0]+sp[1]+sp[2], passed ) ) return false;

    bool pr = true;
    sender_ip = sp[1];
    sender_port = safe_stoi(sp[2], pr);
    tt = sp[3]; 
    dnext= safe_stoi(sp[4], pr);
    tsec = safe_stoull( sp[5], pr );
    mode = safe_stoi( sp[6], pr );

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr  ) ){
      cout << "Could not get proper values of ping"<<endl;
      cout << pr <<  endl;
      return false;
    }

    return true;
}



bool key_present( string key, map<string,int> &passed )
{
	if (passed.find(key) != passed.end() )
    	return true; 

    passed.insert( make_pair( key, 1 ) );
    return false;
} 

//创建形式为string的process_block
string create__process_block( block* bk )
{

    string s = "#process_block,"+my_ip+","+to_string(my_port)+","+to_string(bk->nb->chain_id)+","+to_string(bk->nb->parent)+","+to_string(bk->hash)+",";
    s += to_string(bk->nb->no_txs) + ","; //7
    s += to_string(bk->nb->depth) + ","; //8
    s += to_string(bk->nb->rank) + ","; //9
    s += to_string(bk->nb->next_rank) + ","; //10
    s += to_string(bk->nb->time_mined) + ","; //11
    s += to_string(bk->nb->trailing) + ","; //12
    s += to_string(bk->nb->trailing_id) + ","; //13
    s += bk->nb->merkle_root_chains + ","; //14
    s += bk->nb->merkle_root_txs + ","; //15
    s += to_string(bk->number_of_weakblocks); //16
    //cout << "create__process_block:" <<  bk->number_of_weakblocks << endl;
    for(int i=0; i<bk->nb->proof_new_chain.size(); i++) s += "," + bk->nb->proof_new_chain[i];//17～16+1*MPL
    if(bk->number_of_weakblocks > 0){
      for( int i = 0; i < bk->number_of_weakblocks; i++){
        s += "," + to_string(bk->vec_weakblocks[i]);
      }
    }
    
    return s;
}


bool parse__process_block( vector<std::string> sp, map<string,int> &passed, string &sender_ip, uint32_t &sender_port, network_block &nb , vector<BlockHash>& vec_weakblocks)
{
    int MPL = merkle_proof_length();

    //如果该请求格式不对，则不解析
  	if ( sp.size() < 16 + 1 * MPL + 1 ) return false;
    //如果该请求重复，则不解析
  	if ( key_present( sp[0]+sp[1]+sp[2]+sp[3]+sp[4]+sp[5], passed ) ) return false;

    //将传过来的报文解析为区块的具体值
    bool pr = true;
    sender_ip               = sp[1];
    sender_port             = safe_stoi(    sp[ 2], pr);
    nb.chain_id             = safe_stoi(    sp[ 3], pr);
    nb.parent               = safe_stoull(  sp[ 4], pr );
    nb.hash                 = safe_stoull(  sp[ 5], pr );
    nb.no_txs               = safe_stoi(    sp[ 6], pr);
    nb.depth                = safe_stoi(    sp[ 7], pr );
    nb.rank                 = safe_stoi(    sp[ 8], pr);
    nb.next_rank            = safe_stoi(    sp[ 9], pr );
    nb.time_mined           = safe_stoull(    sp[ 10], pr );
    nb.trailing             = safe_stoull( sp[11], pr );
    nb.trailing_id          = safe_stoull( sp[12], pr );
    nb.merkle_root_chains   = sp[13];
    nb.merkle_root_txs      = sp[14];
    

    uint32_t number_of_weakblocks = safe_stoi(    sp[ 15], pr);
    for(int j = 0; j < MPL; j++)  nb.proof_new_chain.push_back(sp[16+j]);

    //cout << "parse__process_block:" <<  number_of_weakblocks << endl;
    if(number_of_weakblocks > 0){
      for(int i = 0; i < number_of_weakblocks; i++){
        vec_weakblocks.push_back(safe_stoull( sp[16 + i + MPL * 1], pr ));
      }
    }

    if(  PRINT_TRANSMISSION_ERRORS  &&  ! (pr && sender_ip.size()> 0 && nb.chain_id < CHAINS ) ){
          cout << "Could not get proper values of process_block"<<endl;
          cout << pr << " " << sender_ip << " " << nb.chain_id << endl;

          for( int i=1; i<=sp.size(); i++)
            cout << sp[i] << endl;

          return false;
    }

    
    return true;
}