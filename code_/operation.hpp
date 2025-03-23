#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Block.hpp"
#include "Disk.hpp"
#include "Unit.hpp"
#include "Request.hpp"
#include "Object.hpp"

extern Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
extern Request request[MAX_REQUEST_NUM];
extern Disk disk[MAX_DISK_NUM];
extern Object object[MAX_OBJECT_NUM];
extern Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];

bool check_value(int dk_id , int pos){
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    return block[oj_id][bk_id].check();
}

bool predict(int dk_id) {

    int pt = disk[dk_id].point;
    int cost = disk[dk_id].last_take_tokens;
    
    int tot1 = 0, tot2 = 0;
    for(int i = 0;i < next_step;i++) {
        int cur_cost = get_cost(cost);
        tot1 += cur_cost;
        cost = cur_cost;
    }

    bool is_read = false;
    cost = disk[dk_id].last_take_tokens;
    for(int i = 0;i < next_step;i++, pt = pt % V + 1) {
        if(!check_value(dk_id, pt)) {
            tot2 += 1;
            is_read = false;
        } else if(is_read) {
            int cur_cost = get_cost(cost);
            tot2 += cur_cost;
            cost = cur_cost;
        } else {
            cost = 64;
            tot2 += cost;
            is_read = true;
        }
    }

    return tot1 <= tot2;
}

/*
    获取 标签tag 的固定块.
*/

vector<info> check_tag(int tag, int object_size) {
    std::vector<info>res;
    for(int i = 1;i <= N;i++) {
        for(int j = 1;j <= DISK_BLOCK_GU;j++) {
            const auto& dk = disk[i].disk_block_gu[j];
            if(dk.tag == tag && dk.empty_num >= object_size) {
                res.push_back({dk.empty_num, i, j});
            }
        }
    }
    return res;
}

/*
    获取连续块.
    倘若获取成功，那么 返回连续块的位置； 否则，返回 size 个空位置.
*/

pair<vector<int>, int> get_continuos_unit(int disk_id, int st, int ed, int size) {
    
    std::vector<int> res;
    int tar = 0;        //  若成功，tart就是首位置
    
    for(int i = st;i <= ed;i++) {
        if(!unit[disk_id][i].is_exist) {
            if(res.size() < size) {
                res.push_back(i);
            }
            int cur = i, cnt = 0;
            while(cnt < size && !unit[disk_id][cur].is_exist) {
                cnt += 1;
                cur = get_next_position(cur, st, ed);
            }
            if(cnt >= size) {
                tar = i, cur = i, cnt = 0;
                res.clear();
                while(cnt < size) {
                    res.push_back(cur);
                    cnt += 1;
                    cur = get_next_position(cur, st, ed);
                }
                break;
            }
        }
    }

    assert(res.size() >= size);
    return std::make_pair(res, tar);
}

/*
    获取一个空闲的固定块，要求 不存在任何副本 和 已有 tag 固定块尽可能少 (策略而已，不需要可以修改).
*/

pair<int, int> get_empty_block(int tag, const vector<bool>& is_occupied) {

    std::vector<info> a;
    for(int i = 1;i <= N;i++) {
        if(is_occupied[i] == false) {
            for(int j = 1;j <= DISK_BLOCK_GU;j++) {
                if(disk[i].disk_block_gu[j].is_exist == false ) {
                    a.push_back({disk[i].Tag_number[tag], i, j});
                    break;
                }
            }
        }
    }

    std::sort(a.begin(), a.end());
    if(a.empty()) {
        return std::make_pair(0, 0);
    }

    auto [_, disk_id, first_block] = a[0];
    return std::make_pair(disk_id, first_block);
}