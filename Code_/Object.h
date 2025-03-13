#pragma once
#pragma GCC optimize("O2")
#include "utils.h"

class BlockManager{
public:
    
    //  un_set 的插入和删除效率大约为 O(1).
    int id;
    int size;
    vector<un_set>q;
    
    BlockManager() {}
    BlockManager(const int id_, const int size_) {
        id = id_;
        size = size_;
        q.resize(size + 1);
    }

    //  读入一个对象请求...
    void recieve(const int ID) {
        for(int i = 1;i <= size;i++) {
            q[i].insert(ID);
        }
    }

    // 当磁盘读取到第 xth 个对象块, 返回对应请求数组. 
    un_set Read(const int xth) {
        assert(xth >= 1 && xth <= size);
        auto rs = q[xth];
        q[xth].clear();
        return rs;
    }

    void del(const int r_id) {
        for(int i = 1;i <= size;i++) {
            q[i].erase(r_id);
        }
    }

    //  当 删除对象 ID 时，返回没完成的请求
    un_set clean() {
        un_set rs;
        for(int i = 1;i <= size;i++) {
            rs.insert(q[i].begin(), q[i].end());
            q[i].clear();
        }
        return rs;
    }
};

class Object : public BlockManager{
public:
    int id;
    int size;
    int tag;
    int replica[REP_NUM + 1];
    int Unit_id[4][MAX_OBJECT_SIZE];
    bool is_deleted;
    
    Object() {
        is_deleted = false;
    }

    Object(const int id_, const int size_, const int tag_) : 
        BlockManager(id_, size_) {
        id = id_;
        size = size_;
        tag = tag_;
    }

    //  Object 内部仅关注写入和读取策略.
    
};