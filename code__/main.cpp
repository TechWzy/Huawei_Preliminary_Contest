#pragma once
#include "operation.hpp"

Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
Request request[MAX_REQUEST_NUM];
Disk disk[MAX_DISK_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];

int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
std::queue<std::array<int, 2>>request_queue;

int tag_max_number[MAX_TAG_NUM];
int tag_final_number[MAX_TAG_NUM];
int need_block_number[MAX_TAG_NUM];
int tag_allocation[MAX_DISK_NUM][MAX_TAG_NUM];

int read_global_state(Type type, int tag, int kth) {
    const int total = (T - 1) / FRE_PER_SLICING + 1;
    assert(kth >= 1 && kth <= total);
    assert(tag >= 1 && tag <= M);
    return global_state[tag + type * M][kth];
}

int timestamp;
void timestamp_action()
{
    timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);
    fflush(stdout);
}

void delete_action()
{
    int n_delete;
    int abort_num = 0;
    static int _id[MAX_OBJECT_NUM];
    std::set<int> abort_set;

    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++) {
        scanf("%d", &_id[i]);
    }

    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        auto [num, set_] = object[id].deleted();
        abort_num += num;
        abort_set.insert(set_.begin(), set_.end());
    }
    printf("%d\n", abort_num);
    for (int rd_id : abort_set) {
        printf("%d\n", rd_id);
    }
    fflush(stdout);
}


void do_write(int oid,int size,int copy_id,int disk_id, vector<int> block_pos) {
    
    assert(size == block_pos.size());

    printf("%d", disk_id);
    int curcurrent_write_num = 0;
    for(int i = 0;i < block_pos.size();i++) {
        assert(unit[disk_id][block_pos[i]].is_exist == false);
        printf(" %d", block_pos[i]);
        unit[disk_id][block_pos[i]].add_block(oid, i + 1);
    }

    object[oid].set_pos(copy_id, block_pos, disk_id);
}

void write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);
        object[id].set(size, tag);
        printf("%d\n", id);

        std::vector<bool>is_occupied(N + 1);

        //  1. 找到所有 tag 的固定块.
        //  empty_num 优先， 其次是块连续优先.

        int xth = 1;
        auto [res1, res2] = check_tag(tag, size);

        while((res1.size() || res2.size()) && xth <= 3) {
            
            auto [_, disk_id, first_empty_block] = !res1.empty()?res1.back() : res2.back();
            
            if(!res1.empty()) {
                res1.pop_back();
            } else {
                res2.pop_back();
            }

            if(is_occupied[disk_id]) {
                continue ;
            }

            is_occupied[disk_id] = true;
            auto& dk = disk[disk_id].disk_block_gu[first_empty_block];
            auto [block_pos, tar] = get_continuos_unit(disk_id, dk.first_point, dk.first_point + dk.size - 1, size);
            
            do_write(id, size, xth, disk_id, block_pos);
            printf("\n");
            xth += 1;

            disk[disk_id].add_object_gu(id, size, tag, first_empty_block);
        }

        if(xth > 3) {
            continue ;
        }
        
        //  2. 寻找一个空闲的固定块
        //  要求 该磁盘上的 tag 尽可能少 且 不存在任意副本
        
        while(xth <= 3) {
            
            auto [disk_id, first_empty_block] = get_empty_block(tag, is_occupied);
            is_occupied[disk_id] = true;

            std::vector<int>block_pos;
            const auto& dk = disk[disk_id].disk_block_gu[first_empty_block];

            assert(dk.size >= size);
            for(int i = dk.first_point;block_pos.size() < size;i++) {
                block_pos.push_back(i);
            }

            do_write(id, size, xth, disk_id, block_pos);
            printf("\n");
            xth += 1;

            disk[disk_id].add_object_gu(id, size, tag, first_empty_block);
        }

        if(xth > 3) {
            continue ;
        }

        //  3. 添加在随机块.
        //  随机块的读取效率很低， 不强求找到连续快

        for(int i = 1;i <= N && xth <= 3;i++) {
            if(is_occupied[i]) {
                continue ;
            }
            auto& dk = disk[i].disk_block_sui;
            if(dk.empty_num >= size) {   
                auto [block_pos, tar] = get_continuos_unit(i, dk.first_point, dk.first_point + dk.size - 1, size);
                do_write(id, size, xth, i, block_pos);
                printf("\n");
                xth += 1;
                is_occupied[i] = true;
                disk[i].add_object_sui(id, size);
            }
        }

        //  4.没办法了，调参吧！
        assert(xth > 3);
    }

    fflush(stdout);
}

std::vector<int> do_read(int dk_id) {
    std::vector<int> complete_id;
    int pos = disk[dk_id].point;
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    std::set<int> temp_set = block[oj_id][bk_id].requested_id_block;
    for (int rq_id : temp_set) {
        if (request[rq_id].readed(bk_id)) {
            complete_id.push_back(rq_id);
            object[oj_id].requested_id.erase(rq_id);
        }
        block[oj_id][bk_id].requested_id_block.erase(rq_id);
    }
    disk[dk_id].point = (disk[dk_id].point % V) + 1;
    disk[dk_id].last_point_status = READ;
    printf("r");
    fflush(stdout);
    return complete_id;
}

void read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++) {
        scanf("%d%d", &request_id, &object_id);
        object[object_id].add_request(request_id);
        request_queue.push({get_current_time() + EXTRA_TIME, request_id});
    }
    int complete = 0;
    std::vector<int> complete_id;
    for (int i = 1; i <= N; i++) {
        int j;
        bool ok = false;
        for (j = 0; j < G; j++) {
            int temp_pos = disk[i].point + j;
            if (temp_pos > V) temp_pos -= V;
            if (check_value(i, temp_pos)) {
                ok = true;
                break;
            }
        }

        if (ok == false) {
            bool exist_value = false;
            int k;
            for (k = 1; k <= V; k++)
            {
                if (check_value(i, k)) {
                    exist_value = true;
                    break;
                }
            }
            if (exist_value)
            {
                printf("j %d\n", k);
                fflush(stdout);
                disk[i].point = k;
                disk[i].last_point_status = JUMP;
            }
            else {
                printf("#\n");
                fflush(stdout);
            }
        }
        else { 
            bool flag = false;      //  判断是否需要采取连读策略?
            while (disk[i].rest_tokens > 0) {
                if(!check_value(i, disk[i].point)) {
                    if(!flag) {
                        flag = predict(i);
                    }
                } else {
                    flag = false;
                }
                while (disk[i].rest_tokens > 0 && !check_value(i, disk[i].point) && !flag) {
                    disk[i].last_point_status = PASS;
                    disk[i].rest_tokens--;
                    disk[i].last_take_tokens = 1;
                    disk[i].point = (disk[i].point % V) + 1;
                    printf("p");
                    fflush(stdout);
                }
                if (disk[i].rest_tokens == 0) {
                    break;
                }
                int cur_take_tokens = READ_TAKE_TOKENS;
                if (disk[i].last_point_status == READ) {
                    cur_take_tokens = std::max(16, static_cast<int>(std::ceil(disk[i].last_take_tokens * 0.8)));
                }

                if (disk[i].rest_tokens >= cur_take_tokens)
                {
                    disk[i].rest_tokens -= cur_take_tokens;
                    std::vector<int> temp_vector = do_read(i);
                    complete_id.insert(complete_id.begin(), temp_vector.begin(), temp_vector.end());
                    disk[i].last_take_tokens = cur_take_tokens;
                }
                else {
                    break;
                }
                
            }
            printf("#\n");
            fflush(stdout);
        }
        disk[i].rest_tokens = G;

    }
    printf("%d\n", complete_id.size());
    fflush(stdout);
    for (int rq_id : complete_id) {
        printf("%d\n", rq_id);
        fflush(stdout);
    }
}

void init() {
    
    const int total_period = (T - 1) / FRE_PER_SLICING + 1;
    //  初始化 need_block_number[]
    for(int i = 1;i <= M;i++) {
        int cur_number = 0;
        for(int j = 1;j <= total_period;j++) {
            cur_number -= read_global_state(Delete, i, j);
            cur_number += read_global_state(Write, i, j);
            tag_max_number[i] = std::max(tag_max_number[i], cur_number);
        }
        tag_final_number[i] = cur_number;
    }

    assert(V >= DISK_BLOCK_SUI_SIZE);
    const int block_size = (V - DISK_BLOCK_SUI_SIZE) / DISK_BLOCK_GU;

    for(int i = 1;i <= M;i++) {
        int need_number = (0.3 * tag_max_number[i] + 0.7 * tag_final_number[i]) * 3;
        need_block_number[i] = need_number / block_size;    //  下取整避免浪费.
    }

    for(int i = 1;i <= M;i++) {
        need_block_number[i] = std::min(need_block_number[i], 3);
    }

    int sum = 0;
    for(int i = 1;i <= M;i++) {
        sum += need_block_number[i];
    }

    assert(sum <= N * DISK_BLOCK_GU);

    /*
        将固定块划分到磁盘，每一个标签所划分的磁盘数量依次为: 4, 5, 6
    */

    std::vector<std::array<int, 2>>Tag_sort;
    std::vector<std::array<int, 2>> tg;
    std::vector<int>disk_block_num(N + 1, DISK_BLOCK_GU);

    for(int i = 1;i <= M;i++) {
        Tag_sort.push_back({need_block_number[i], i});
    }
    std::sort(Tag_sort.rbegin(), Tag_sort.rend());
    
    for(int i = 0;i < Tag_sort.size();i++) {
        auto [num, tag_id] = Tag_sort[i];
        int disk_num = (i <= M / 3)?6 : (i <= 2 * M / 3)?5 : 4;
        disk_num = std::min(num, disk_num);
        tg.insert(tg.end(), num % disk_num, {num / disk_num + 1, tag_id});
        tg.insert(tg.end(), num - num % disk_num, {num / disk_num, tag_id});      
    }

    std::sort(tg.begin(), tg.end());
    std::vector<int>vis(M + 1);
    std::vector<int>flag(tg.size());

    //  确保一定能分配成功!
    for(int i = 1;i <= N;i++) {
        int res = DISK_BLOCK_GU;
        for(int j = tg.size() - 1;j >= 0;j--) {
            if(!flag[j]) {
                auto [need, tag_id] = tg[j];
                if(!vis[tag_id] && res >= need) {
                    vis[tag_id] = 1;
                    flag[j] = 1;
                    res -= need;
                    tag_allocation[i][tag_id] = need;
                }
            }
        }
        for(int j = 1;j <= M;j++) {
            vis[j] = 0;
        }
    }

    bool ok = true;
    for(int i = 0;i < tg.size();i++) {
        ok &= flag[i];
    }

    assert(ok == true);

    //  磁盘初始化
    for(int i = 1;i <= N;i++) {
        disk[i].init(V, G, i);
    }
}

int main()
{

    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);

    for (int i = 1; i <= 3 * M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d", &global_state[i][j]);
        }
    }

    init();
  
    put_ok();
    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        get_current_time(1);
        delete_action();
        write_action();
        read_action();
    }

    return 0;
}