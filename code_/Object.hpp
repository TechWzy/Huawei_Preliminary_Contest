#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Block.hpp"
#include "Request.hpp"
#include "Unit.hpp"

extern Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
extern Request request[MAX_REQUEST_NUM];
extern Disk disk[MAX_DISK_NUM];
extern Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];
extern Object object[MAX_OBJECT_NUM];

class Object {
public:
	int id;
	int size = 0;
	int tag = 0;
	int replica[REP_NUM + 1] = { 0 };   //每个副本所在磁盘编号
	int r_replica[MAX_DISK_NUM] = { 0 };//磁盘编号所存的副本
	int unit_id[REP_NUM + 1][MAX_OBJECT_SIZE] = { 0 }; //具体块的位置
	std::set<int> requested_id; //请求这个对象的请求id
	bool is_deleted = false;    //是否被删除
	static int cnt;
    Object() {
        id = cnt++;
    }

	// 删除该对象
    std::pair<int, std::set<int>> deleted() {
        // 对应删除磁盘上的对象
        for (int i = 1; i <= REP_NUM; i++) {
            int first_pos = unit_id[i][1];
            disk[replica[i]].delete_object(id, size, first_pos);
        }

        // 对应删除单元上的对象
        for (int i = 1; i <= REP_NUM; i++) {
            for (int j = 1; j <= size; j++) {
                unit[replica[i]][unit_id[i][j]].deleted();
            }
        }

        // 对应删除块
        for (int i = 1; i <= size; i++) {
            block[id][size].deleted();
        }

        // 对应删除请求
        for (int rd_id : requested_id) {
            request[rd_id].deleted();
        }

        int abort_num = requested_id.size();
        std::set<int> temp_requested_id = requested_id;
        size = 0;
        tag = 0;
        requested_id.clear();
        memset(replica, 0, sizeof(replica));
        memset(r_replica, 0, sizeof(r_replica));
        memset(unit_id, 0, sizeof(unit_id));
        is_deleted = true;

        return { abort_num, temp_requested_id };
    }

    void set(int size, int tag)
    {
        this->size = size;
        this->tag = tag;
    }

    void set_pos(int copy_id, std::vector<int>& bk_pos, int dk_id){
        replica[copy_id] = dk_id;
        for (int i = 1; i <= size; i++) {
            unit_id[copy_id][i] = bk_pos[i - 1];
            block[id][i].set(copy_id, bk_pos[i - 1], dk_id, tag);
        }
    }

    void add_request(int rd_id) {
        requested_id.insert(rd_id);
        request[rd_id].create(id, size);
        for (int i = 1; i <= size; i++) {
            block[id][i].add_request_block(rd_id);
        }
    }
};
int Object::cnt = 0;