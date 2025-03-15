#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"

class Unit {
public:
	int disk_id; 
	int object_id = 0; 
	int block_order = 0;
	int id; 
	bool is_exist = false; 
	static int cnt;

	Unit() {
		disk_id = cnt / MAX_DISK_SIZE;
		id = cnt % MAX_DISK_SIZE;
		cnt++;
	}

	void deleted() {

		is_exist = false;
		object_id = 0;
		block_order = 0;
	}

	void add_block(int oj_id, int bk_order)
	{
		is_exist = true;
		object_id = oj_id;
		block_order = bk_order;
	}

};

int Unit::cnt = 0;