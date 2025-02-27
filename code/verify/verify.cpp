
#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/sha.h>
#include "../data/Blockchain.hpp"
#include "../utils/crypto_stuff.h"

using namespace std;


string blockhash_to_string( BlockHash b)
{
	stringstream sstream;
	sstream << hex << b;
	string hash = sstream.str();
	
	if ( hash.length() < 2*sizeof(BlockHash) )
		hash = string(2*sizeof(BlockHash)-hash.length(), '0').append( hash );
	return hash;
}

BlockHash string_to_blockhash( string h ){

	return stoull( h.substr(0, 2*sizeof(BlockHash)), nullptr, 16);
}


uint32_t get_chain_id_from_hash( string h)
{
	return stoi ( h.substr(58) ,nullptr,16) % CHAINS;
}




string compute_merkle_tree_root( vector<string> leaves )
{
	vector<string> tmp = leaves;
	unsigned int next = 0;
	while( tmp.size() > 1 ){
		vector <string> tmp2 = tmp;
		tmp.clear();
		for( int i=0; i< tmp2.size()/2; i++){
			string st = tmp2[2*i+0] + tmp2[2*i+1];
			next += 2;
			tmp.push_back( sha256( st ) );
		}
	}
	return tmp[0];
}//求merkle根

vector <string> compute_merkle_proof( vector<string> leaves, int index )
{
	int first_index = index;

	vector<string> proof;
	proof.push_back( leaves[index] );
	vector<string> tmp = leaves;
	while( tmp.size() > 1 ){
		vector <string> tmp2 = tmp;
		tmp.clear();
		int adj_index = (index % 2 ) ? (index -1 ) : (index + 1);
		for( int i=0; i< tmp2.size()/2; i++){

			string st = tmp2[2*i+0] + tmp2[2*i+1];
			tmp.push_back( sha256(st) );

			if ( 2*i+0 == adj_index || 2*i+1 == adj_index  )
				proof.push_back( (2*i+0 == adj_index) ? tmp2[2*i+0] : tmp2[2*i+1] );
			
		}
		index /= 2;
	}

	proof.push_back( tmp[0] );



	return proof;
}


int merkle_proof_length()
{
	return 1 + 1*ceil( log(CHAINS) / log(2) ) + 1  ;

}


bool verify_merkle_proof( vector <string> proof, BlockHash bh, string root, uint32_t index )
{


	string h = blockhash_to_string( bh );
	if ( proof[0] != h && proof[1] != h)	return false;

	int i = 1;
	while( i+1 < proof.size() ){

		if ( index % 2) h = sha256( proof[i] + h );
		else h = sha256( h + proof[i] );

		i ++;
		index /= 2;
	}


	if ( proof[i] != h  || root != h ){
		cout <<"bad root"<<endl;
		cout << (proof[i] == h) <<endl;
		cout << (root==h) << endl;
		cout << proof[i] << endl;
		cout << h << endl;
		cout << root << endl;
		return false;
	}


	return true;
}


