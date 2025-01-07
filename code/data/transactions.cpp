

#include "./transactions.h"
#include "../utils/crypto_stuff.h"
#include "../utils/misc.h"

using namespace std;



string get_random_address( uint32_t size_in_dwords )
{
	stringstream sstream;
	for( int i=0; i<size_in_dwords; i++)
		sstream << setfill('0') << setw(8) << hex << rng();

	return sstream.str();
}

string create_one_transaction()
{
	if( fake_transactions ){
	    return "0000000000000000000000000000000000000000:0000000000000000000000000000000000000000:0000000000:00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	}

	string tx = get_random_address(ADDRESS_SIZE_IN_DWORDS) + ":" + get_random_address(ADDRESS_SIZE_IN_DWORDS) + ":" + to_string(rng() );
	string sign_tx = sign_message( tx );

	return tx +":"+sign_tx;
}//由于交易是模拟的，所以内容都是用随机数填充的

int create_transaction_block( BlockHash hash, string filename )
{

	uint32_t l = 0, no_txs = 0;
	
	if ( WRITE_BLOCKS_TO_HDD ){
		ofstream file;
		try{ file.open(filename); }
		catch(const std::string& ex){  return false; }
		while ( l < BLOCK_SIZE_IN_BYTES){
			string tx = create_one_transaction();
			file << tx << endl;
			l += tx.size();
			no_txs ++;
		}
		file.close();
	}
	else{
		while ( l < BLOCK_SIZE_IN_BYTES){
			string tx = create_one_transaction();
			l += tx.size();
			no_txs ++;
		}
	}

	return no_txs;
	

}

bool verify_transaction( string tx )
{

	vector <string > s = split ( tx, ":");
	if( s.size() == 4){
		string ad1 = s[0];
		string ad2 = s[1];
		string amount = s[2];
		string signature = s[3];


		if ( ad1.size() != 8*ADDRESS_SIZE_IN_DWORDS || ad2.size() != 8*ADDRESS_SIZE_IN_DWORDS || amount.size() <= 0 ) 
			return false;

		string full = ad1+":"+ad2+":"+amount;

		return verify_message( full, signature);
	}
	else{
		if ( PRINT_TRANSMISSION_ERRORS ){
			cout<<"Incorrect transaction size:"<<s.size()<<endl;
			cout <<"tx:"<<tx<<endl;
		}
		return false;
	}
}