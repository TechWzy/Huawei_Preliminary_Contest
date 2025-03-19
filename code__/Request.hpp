#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"

class Object;

class Request {
public:
	int id;
	int object_id = 0;
	int object_size = 0;
	int rest_block_num = 0;
	bool is_deleted = false; 
	static int cnt;
	Request() {
		id = cnt++;
	}

	void deleted() {
		object_id = 0;
		object_size = 0;
		rest_block_num = 0;
		is_deleted = true;
	}

	void create(int oj_id, int oj_size) {
		object_id = oj_id;
		object_size = oj_size;
		rest_block_num = oj_size;

		is_deleted = false;
	}
	
	bool readed(int bk_order) {
		if (rest_block_num == 1) {
			rest_block_num = 0;
			object_id = 0;
			object_size = 0;
			is_deleted = true;
			return true;
		}
		else {
			rest_block_num--;
			return false;
		}
	}
};

int Request::cnt = 0;



