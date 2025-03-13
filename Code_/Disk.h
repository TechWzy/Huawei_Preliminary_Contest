#pragma once
#pragma GCC optimize("O2")
#include "utils.h"

class Unit{
public:
    int object_id;
    int block_order;
    bool is_exist;

    Unit() {
        is_exist = true;
        object_id = 0;
        block_order = 0;
    }
};

class Disk{
public:
    
    using un_map = unordered_map<int, bool>;
    int id;
    int empty_num;
    int point;
    int object_num;
    int first_empty;      
    int size;
    un_map object_id;
    vector<Unit> units;

    Disk() {
        point = 1; 
        first_empty = 1;
        object_num = 0;
    }

    Disk(const int id_, const int size_) {
        
        point = 1; 
        first_empty = 1;
        object_num = 0;
        
        id = id_;
        size = size_;
        empty_num = size;
        units.resize(size + 1);
    }

    void SetEmpty(const int uid) {
        assert(uid >= 1 && uid <= size);
        units[uid].is_exist = true;
        units[uid].object_id = 0;
        units[uid].block_order = 0;  
    }

};