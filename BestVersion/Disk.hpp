#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Unit.hpp"

extern Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
extern int tag_unit_number[MAX_TAG_NUM];
enum POINT_STATUS { PASS, JUMP, READ };

class DiskBlock {
public:

    int tag = 0;
    int size = 0;
    int empty_num = 0;      //  empty_num 改为记录空余存储单元的数量.
    int first_point = 0;
    bool Is_Permanent = false;

    DiskBlock() {}
    DiskBlock(int first_point, int block_size, int empty_num, int tag, bool is_parmanent = false) {
        this->first_point = first_point;
        this->empty_num = empty_num;
        this->Is_Permanent = is_parmanent;
        this->tag = tag;
        size = block_size;
    }

    void add_unit() {
        empty_num -= 1;
        tag_unit_number[tag] -= 1;
    }

    void delete_unit() {
        empty_num += 1;
        tag_unit_number[tag] += 1;
    }
};

class Disk {
public:
    
    int blockIndex = 0;
    int id = 0;
    int size = 0;
    int empty_num = 0;
    int point = 1;
    int rest_tokens = 0;
    int last_take_tokens = 0;
    POINT_STATUS last_point_status = PASS;
    std::set<info> infos;
    std::map<int, DiskBlock> block;
    int tag_block_number[MAX_TAG_NUM];
    
    Disk() {}

    void set(int V, int id) {
        this->id = id;
        this->size = V;
        this->empty_num = V;
    }

    //  获取该点所处的动态块
    info Get_block_info(int x) {
        
        assert(x >= 1 && x <= V);
        if(infos.empty()) {
            return info {0, 0, 0};
        } 

        auto it = infos.lower_bound(info{x, 0, 0});
        if(it != infos.end() && (*it)[0] == x) {
            return *it;
        }
        
        if(it != infos.begin()) {
            it = std::prev(it);
        }
        
        auto [L, R, _] = *it;
        if(L <= x && x <= R) {
            return *it;
        }
        
        return info {0, 0, 0};
    }

    //  判断线段[l, r] 是否已和现有的block对应的[l, r] 冲突.
    bool Is_Intersect(int l, int r) {
        
        auto [l1, r1, id1] = Get_block_info(l);
        auto [l2, r2, id2] = Get_block_info(r);
        
        if(l1 || l2) {
            return true;
        }

        auto it = infos.lower_bound({l, 0, 0});
        if(it != infos.end()) {
            auto [L, R, _] = *it;
            if(l <= L && L <= r) {
                return true;
            }
        }

        return false;
    }

    bool allocate_block(int l, int r, int tag, bool is_parmanent = false) {
        
        assert(r >= l && tag > 0);
        if(Is_Intersect(l, r)) {    //  和现有block冲突，分配失败
            return false;
        }

        int empty_num = 0;
        const auto& un = unit[id];
        for(int i = l;i <= r;i++) {
            empty_num += (un[i].is_exist == false);
        }

        tag_unit_number[tag] += (r - l + 1);    //  默认该区域的其他标签不存在.
        auto index = ++blockIndex;
        infos.insert(info {l, r, index});
        block[index] = DiskBlock(l, r - l + 1, empty_num, tag, is_parmanent);

        if(!is_parmanent) {
            tag_block_number[tag] += 1;
        }

        return true;        //  分配成功.
    }

    void delete_block(int index) {
        assert(block.count(index) > 0);
        auto bl = block[index];
        int L = bl.first_point, R = bl.first_point + bl.size - 1;
        tag_unit_number[bl.tag] -= (R - L + 1);

        if(bl.Is_Permanent == false) {
            tag_block_number[bl.tag] -= 1;    
        }

        infos.erase({L, R, index});
        block.erase(index);
    }

    void delete_unit(int position) {
        empty_num += 1;
        auto [L, R, Index] = Get_block_info(position);
        if(L != 0 && block.count(Index)) {
            block[Index].delete_unit();
        }
    }

    void add_unit(int position) {
        empty_num -= 1;
        auto [L, R, Index] = Get_block_info(position);
        assert(L != 0 && block.count(Index) > 0);
        block[Index].add_unit();
    }

    //  op == 1 表示获取 位置 x 的标签
    //  op == 0 表示获取 index = x 的标签.
    int get_tag(int x, int op = 0) {
        if(op) {
            auto [l, r, _] = Get_block_info(x);
            x = _;
        }
        return (x?block[x].tag : 0);
    }

    //  获取可填写的区域
    vector<array<int, 2>> get_available_block() {
        
        int ls = 0;
        std::vector<std::array<int, 2>>avail;
        for(auto [l, r, _] : infos) {
            int L = ls + 1, R = l - 1;
            if(L <= R) {
                avail.push_back({L, R});
            }
            ls = r;
        }
        
        if(ls < size) {
            avail.push_back({ls + 1, size});            
        }

        return avail;
    }
};