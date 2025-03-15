#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Unit.hpp"


enum POINT_STATUS { PASS, JUMP, READ };

class DiskBlock {
public:
    int tag = 0;
    bool is_exist = false;
    int size = 0;
    int empty_num = 0;
    int first_point = 0;
    int lei = 0;
    void first_set(int first_point,int block_size) {
        this->first_point = first_point;
        size = block_size;
        empty_num = size;
    }
    void set_tag(int tag) {
        this->tag = tag; 
        is_exist = true;
    }
    void add_object(int oj_size) {
        empty_num -= oj_size;
    }
    void deleted(int oj_size) {
        empty_num += oj_size;
        lei += oj_size;
        // 磁盘块上的全部对象被删除了
        if (empty_num==size) {
            tag = 0;
            lei = 0;
            is_exist = false;
        }
    }
};

class Disk {
public:
    int id;
    int size = 0;
    int empty_num = 0;
    int point = 1;
    int object_num = 0;
    int rest_tokens = 0;
    int tokens = 0;
    int block_size = 0;
    POINT_STATUS last_point_status = PASS;
    int last_take_tokens = 0;
    DiskBlock disk_block_gu[DISK_BLOCK_GU];
    DiskBlock disk_block_sui; // 除去固定，剩余全是随机
    int gu_size = 0;
    static int cnt;
    std::unordered_map<int, int> ma;
    Disk() {}

    void set(int V, int G) {
        size = V;
        empty_num = V;
        rest_tokens = G;
        tokens = G;
        block_size = size / (DISK_BLOCK_NUM - 1);
        int first_point = 1;
        for (int i = 1; i <= DISK_BLOCK_GU - 1; i++) {
            disk_block_gu[i].first_set(first_point,block_size);
            first_point += block_size;
            gu_size += block_size;
        }
        // 计算随机块信息
        disk_block_sui.first_set(first_point, size - first_point + 1);

    }
    void delete_object(int oj_id, int object_size) {
        empty_num += object_size; // 空单元数量增加
        object_num--; // 对象数量减少
        if (ma[oj_id] != -1) {
            disk_block_gu[ma[oj_id]].deleted(object_size);
        }
        else {
            disk_block_sui.deleted(object_size);
        }
        ma.erase(oj_id);
    }

    // 检查标签是否在这个磁盘中有固定块，且够存
    std::pair<int,int> check_tag(int tag,int oj_size) {
        for (int i = 1; i <= DISK_BLOCK_GU - 1; i++) {
            if (disk_block_gu[i].tag == tag) {
                if (disk_block_gu[i].empty_num < oj_size) {
                    continue;
                }
                return { disk_block_gu[i].first_point,i };
            }
        }
        return {0,0};
    }


    // 这个磁盘想要写空块，有没有机会写
    std::pair<int,int> disk_want_write_gu(int tag,int oj_size) {
        int empty_block = 0;
        for (int i = 1; i <= DISK_BLOCK_GU - 1; i++) {

            //返回第一个空块
            if (!disk_block_gu[i].is_exist&&!empty_block) {
                empty_block = i;
                return { disk_block_gu[empty_block].first_point ,empty_block };
            }
        }
        return{ 0,0 };
    }


    int disk_want_write_sui(int oj_size) {
        if (disk_block_sui.empty_num < oj_size) {
            return false;
        }
        return disk_block_sui.first_point;
    }


    // 加一个固态块对象
    void add_object_gu(int oj_id,int oj_size, int tag, int dk_bk_id) {
        empty_num -= oj_size;
        object_num++;
        ma[oj_id] = dk_bk_id;
        if (!disk_block_gu[dk_bk_id].is_exist) {
            disk_block_gu[dk_bk_id].set_tag(tag);
        }
        disk_block_gu[dk_bk_id].add_object(oj_size);
    }

    // 加一个随机块对象
    void add_object_sui(int oj_id,int oj_size) {
        empty_num -= oj_size;
        object_num++;
        ma[oj_id] = -1;
        disk_block_sui.add_object(oj_size);
    }
};

int Disk::cnt = 0;



