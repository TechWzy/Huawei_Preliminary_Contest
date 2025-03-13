#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Unit.hpp"



enum POINT_STATUS { PASS, JUMP, READ };

class Disk {
public:
    int id;
    int size = 0;
    int empty_num = 0;
    int point = 1;
    int object_num = 0;
    std::set<int> object_id;
    int rest_tokens = 0;
    int tokens = 0;
    int first_empty = 1;
    POINT_STATUS last_point_status = PASS;
    int last_take_tokens = 0;
    static int cnt;

    Disk() {
        id = cnt++;
    }
    void delete_object(int oj_id, int object_size, int first_pos) {
        empty_num += object_size; // 空单元数量增加
        object_num--; // 对象数量减少
        first_empty = std::min(first_empty, first_pos); // 更新该磁盘第一个空块位置
        object_id.erase(oj_id);
    }

    // 初始化磁盘数值
    void set(int V, int G){
        size = V;
        empty_num = V;
        tokens = G;
        rest_tokens = G;
    }

    // 加一个对象
    int add_object(int oj_id, int object_size){
        if (empty_num >= object_size) {
            empty_num -= object_size; // 空单元数量减少
            object_num++; // 对象数量增加
            object_id.insert(oj_id);
            return first_empty;
        }
        else {
            return 0;
        }    
    }
};

int Disk::cnt = 0;



