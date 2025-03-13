#pragma once
#pragma GCC optimize("O2")
#include "utils.h"

class Request {
public:

    int r_id;
    int o_id;
    int size;
    int res_block_size;
    bool is_deleted;
    bool is_sumbitted;
    vector<int>is_read; // is_read[i] : 第 i 个对象块是否被读取?  

    Request() {}
    Request(const int r_id_, const int o_id_, const int size_) {
        
        r_id = r_id_;
        o_id = o_id_;
        size = size_;
        
        res_block_size = size;
        is_deleted = false;
        is_sumbitted = false;
        is_read.assign(size + 1, 0);
    }

    int del() {
        //  -1 : 标志删除失败
        if(is_sumbitted || is_deleted) {
            return -1;
        }
        is_deleted = true;
        return r_id;
    }

    //  返回 true 表示已该请求已完成
    bool recieve(const int block_id) {
        if(is_sumbitted || is_deleted || is_read[block_id]) {
            return false;
        }
        is_read[block_id] = 1;
        res_block_size--;
        if(!res_block_size) {
            is_sumbitted = true;
        }
        return res_block_size == 0;
    }
};