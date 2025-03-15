#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Unit.hpp"
#include "Request.hpp"
#include "Disk.hpp"
#include "Object.hpp"
#include "Segmentree.hpp"

extern Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
extern Request request[MAX_REQUEST_NUM];
extern Disk disk[MAX_DISK_NUM];
extern Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];
extern Object object[MAX_OBJECT_NUM];
extern Segmentree seg[MAX_DISK_NUM];
extern queue<array<int, 2>>q;

void add_request_to_segmentree(int rid) {
    int oid = request[rid].object_id;
    const double bs = 0.5 * (object[oid].size + 1) / object[oid].size;
    q.push({request[rid].ocur_time + EXIST_TIME, rid});
    for(int i = 1;i <= REP_NUM;i++) {
        int disk_id = object[oid].replica[i];
        for(int j = 1;j <= object[oid].size;j++) {
            int uid = object[oid].unit_id[i][j];
            seg[disk_id].add(1, uid, bs);
        }
    }
}

void deleted_request(int rid, int order = 0) {
    int res = current_time() - request[rid].ocur_time;
    int oid = request[rid].object_id;
    const double bs = 0.5 * (object[oid].size + 1) / object[oid].size;
    auto& req = request[rid];
    for(int i = 1;i <= object[oid].size;i++) {
        if(req.vis[i] == false && (!order || i == order)) {
            req.vis[i] = true;
            for(int j = 1;j <= REP_NUM;j++) {
                int disk_id = object[oid].replica[j];
                int uid = object[oid].unit_id[j][i];
                seg[disk_id].modify(1, uid, bs, res);
            }
        }
    }
}

// 检查磁盘位置是否有价值
bool check_value(int dk_id , int pos)
{
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    return block[oj_id][bk_id].check();
}

std::vector<int> do_read(int dk_id) {
    std::vector<int> complete_id;
    int pos = disk[dk_id].point;
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    std::set<int> temp_set = block[oj_id][bk_id].requested_id_block;
    for (int rq_id : temp_set) {
        // 检查可能完成的请求每一次
        if (request[rq_id].readed(bk_id)) {
            complete_id.push_back(rq_id);
            object[oj_id].requested_id.erase(rq_id);
        }
        deleted_request(rq_id, bk_id);
        block[oj_id][bk_id].requested_id_block.erase(rq_id);
    }
    disk[dk_id].point = (disk[dk_id].point % V) + 1;
    disk[dk_id].last_point_status = READ;
    printf("r");
    fflush(stdout);
    return complete_id;
}