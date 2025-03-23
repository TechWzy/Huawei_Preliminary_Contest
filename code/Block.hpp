#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"



class Block {
public:
	int object_tag = 0; 
	int replica_block[REP_NUM + 1] = {0}; 
	int unit_block[REP_NUM + 1] = {0}; 
	std::set<int> requested_id_block; 
	bool is_deleted = false; 
	static int cnt;
	Block() {
		
	}


	void deleted() {
		object_tag = 0;
		memset(replica_block, 0, sizeof(replica_block));
		memset(unit_block, 0, sizeof(unit_block));
		requested_id_block.clear();
		is_deleted = true;
	}

	void set(int copy_id, int pos,int dk_id,int tag){
		replica_block[copy_id] = dk_id;
		unit_block[copy_id] = pos;
		object_tag = tag;
		is_deleted = false;
	}

	void add_request_block(int rd_id) {
		requested_id_block.insert(rd_id);
	}

	bool check()
	{
		return !requested_id_block.empty();
	}
};
int Block::cnt = 0;
