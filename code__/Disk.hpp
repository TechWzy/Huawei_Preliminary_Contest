#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"
#include "Unit.hpp"

extern Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
extern int tag_allocation[MAX_DISK_NUM][MAX_TAG_NUM];

enum POINT_STATUS { PASS, JUMP, READ };

class DiskBlock {
public:

    bool is_Permanent = false;      //  是否为永久块?
    int tag = 0;
    bool is_exist = false;
    int size = 0;
    int empty_num = 0;
    int first_point = 0;

    void first_set(int first_point,int block_size, int tag, bool is_Parmanent) {
        this->first_point = first_point;
        this->tag = tag;
        this->is_Permanent = is_Parmanent;
        size = block_size;
        empty_num = size;
        is_exist = is_Parmanent;
    }
    
    void set_tag(int tag) {
        if(!is_Permanent) {
            this->tag = tag;
            is_exist = true;
        }
    }

    void add_object(int oj_size) {
        empty_num -= oj_size;
    }
    
    void deleted(int oj_size) {
        empty_num += oj_size;
        if (!is_Permanent && empty_num == size) {
            tag = 0;
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
    DiskBlock disk_block_gu[DISK_BLOCK_GU + 1];
    DiskBlock disk_block_sui;
    int Tag_number[MAX_DISK_SIZE]; 
    int gu_size = 0;
    static int cnt;

    //  对象 oid 存储在 ma[oid] 对象块
    std::unordered_map<int, int> ma;    
    Disk() {}

    void init(int V, int G, int disk_id) {
        
        id = disk_id;
        size = V;
        empty_num = V;
        rest_tokens = G;
        tokens = G;
        block_size = block_size = (V - DISK_BLOCK_SUI_SIZE) / DISK_BLOCK_GU;
        
        int first_point = 1, xth = 1;
        for(int i = 1;i <= M;i++) {
            for(int j = 1;j <= tag_allocation[id][i];j++) {
                disk_block_gu[xth++].first_set(first_point, block_size, i, true);
                first_point += block_size;
                Tag_number[i] += 1;
            }
        }
        
        for(int i = xth;i <= DISK_BLOCK_GU;i++) {
            disk_block_gu[i].first_set(first_point, block_size, 0, false);
            first_point += block_size;
        }

        //  确保随机块
        disk_block_sui.first_set(first_point, size - first_point + 1, 0, false);
    }

    void delete_object(int oj_id, int object_size) {
        empty_num += object_size; 
        object_num--;
        if (ma[oj_id] != -1) {
            int tag = disk_block_gu[ma[oj_id]].tag;
            disk_block_gu[ma[oj_id]].deleted(object_size);
            if(disk_block_gu[ma[oj_id]].is_exist == false) {
                Tag_number[tag] -= 1;
            }
        }
        else {
            disk_block_sui.deleted(object_size);
        }
        ma.erase(oj_id);
    }

    std::pair<int,int> disk_want_write_gu(int tag,int oj_size) {
        int empty_block = 0;
        for (int i = 1; i <= DISK_BLOCK_GU - 1; i++) {
            if (!disk_block_gu[i].is_exist && !empty_block) {
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

    void add_object_gu(int oj_id,int oj_size, int tag, int dk_bk_id) {
        empty_num -= oj_size;
        object_num++;
        ma[oj_id] = dk_bk_id;
        //  首次添加需要指定 tag
        if (!disk_block_gu[dk_bk_id].is_exist) {
            disk_block_gu[dk_bk_id].set_tag(tag);
            Tag_number[tag] += 1;
        }
        disk_block_gu[dk_bk_id].add_object(oj_size);
    }

    void add_object_sui(int oj_id,int oj_size) {
        empty_num -= oj_size;
        object_num++;
        ma[oj_id] = -1;
        disk_block_sui.add_object(oj_size);
    }
};

int Disk::cnt = 0;



