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
    判断对象可否写入 large_block?
*/

int Is_LargeBlock_available(int disk_id, int tag, int size) {
    
    auto& dk = disk[disk_id];
    for(auto [l, r, index] : dk.infos) {
        if(dk.block[index].Is_Permanent && dk.block[index].empty_num >= size && dk.block[index].tag == tag) {
            return index;
        }
    }

    return 0;
}

/*
    获取连续块.
    倘若获取成功，那么 返回连续块的位置； 否则，返回 size 个空位置.
*/

pair<vector<int>, int> get_continuos_unit(int disk_id, int st, int ed, int size) {
    
    std::vector<int> res;
    int tar = 0;        //  若成功，tar就是首位置
    
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
    获取已存在的标签块
    优先选中 empty_num 较小的块 （一种提升存储密度的策略）
*/

vector<info> get_existed_block(int tag, const vector<bool>& is_occupied, int size) {
    
    std::vector<info> res;
    for(int i = 1;i <= N;i++) {
        if(is_occupied[i] == false) {
            std::vector<info> avail;
            auto& dk = disk[i];
            for(auto [l, r, index] : dk.infos) {
                auto& bl = dk.block[index];
                if(bl.tag == tag && bl.empty_num >= size) {
                    avail.push_back(info {bl.empty_num, i, index});
                }
            }
            std::sort(avail.rbegin(), avail.rend());
            if(avail.empty() == false) {
                res.push_back(avail.back());
            }
        }
    }

    return res;
}

/*
    获取一个空闲块.
    优先选择最优的空闲块.
*/

info get_empty_block(int tag, int size, const vector<bool>& is_occupied) {

    std::vector<std::array<int, 2>>U;
    for(int i = 1;i <= N;i++) {
        if(is_occupied[i] == false) {
            U.push_back({disk[i].empty_num, i});
        }
    }
    std::sort(U.rbegin(), U.rend());

    //  1. 优先选择最优的空闲块，即标签是连续的.
    for(auto [_, i] : U) {
        auto& dk = disk[i];
        if(dk.tag_block_number[tag] < 4) {
            int ls = 0, last_tag = 0;
            for(auto [l, r, index] : dk.infos) {
                int L = ls + 1, R = l - 1;
                int cur_tag = dk.block[index].tag;
                if(R - L + 1 >= size && (last_tag == tag || cur_tag == tag)) {
                    return info {i, L, R};
                }
                ls = r, last_tag = cur_tag;
            }
            int L = ls + 1, R = V;
            if(R - L + 1 >= size && last_tag == tag) {
                return info {i, L, R};
            }
        }
    }

    //  2. 假如存在连续空块 bl1, bl2 那么优先选择 bl2.
    for(auto [_, i] : U) {
        auto& dk = disk[i];
        if(dk.tag_block_number[tag] >= 4) continue ;
        std::vector<std::array<int, 2>>Seg;
        int ls = 0;
        for(auto [l, r, index] : dk.infos) {
            int L = ls + 1, R = l - 1;
            if(R - L + 1 >= size) {
                Seg.push_back({L, R});
            }
            ls = r;
        }
        if(ls < V && V - ls >= size) {
            Seg.push_back({ls + 1, V});
        }
        for(int j = 1;j < Seg.size();j++) {
            if(Seg[j][0] == Seg[j - 1][1] + 1) {
                return info {i, Seg[j][0], Seg[j][1]}; 
            }
        }
    }

    //  3. 不管了， 随便选一个空块
    for(auto [_, i] : U) {
        auto& dk = disk[i];
        int ls = 0;
        for(auto [l, r, index] : dk.infos) {
            int L = ls + 1, R = l - 1;
            if(R - L + 1 >= size) {
                return info {i, L, R};
            }
            ls = r;
        }
        if(V - ls >= size) {
            return info {i, ls + 1, V};
        }
    }

    return {0, 0, 0};
}

/*
    写入标签tag，要求所选区域的空余块尽可能少.
*/

info WriteIn_Other_Block(int tag, int size, const vector<bool>& is_occupied) {
    
    int cur_empty_num = V, disk_id = 0, index = 0;
    for(int i = 1;i <= N;i++) {
        if(is_occupied[i] == false) {
            for(auto [L, R, Index] : disk[i].infos) {
                auto& bl = disk[i].block[Index];
                if(bl.tag == tag && bl.empty_num >= size) {
                    if(bl.empty_num < cur_empty_num) {
                        cur_empty_num = bl.empty_num;
                        disk_id = i;
                        index = Index;
                    }
                }
            }
        }
    }

    if(cur_empty_num == V) {
        return info {0, 0, 0};
    }

    return info {cur_empty_num, disk_id, index};
}