#pragma once
#include"operation.hpp"

Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
Request request[MAX_REQUEST_NUM];
Disk disk[MAX_DISK_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];

int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];

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


void do_write(int oid,int size,int copy_id, int dk_id, vector<int>block_pos) {
    printf("%d", dk_id);
    for(int i = 0;i < block_pos.size();i++) {
        printf(" %d", block_pos[i]);
        unit[dk_id][block_pos[i]].add_block(oid, i + 1);
    }
    object[oid].set_pos(copy_id, block_pos, dk_id);
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
        
        int xth = 1;
        std::vector<bool>is_occupied(N + 1);

        //  1. 寻找固定块.
        auto tag_block = check_tag(tag, size);
        //  std::sort(tag_block.rbegin(), tag_block.rend());    //  优先使用剩余块较少的固定块
        
        while(tag_block.size() && xth <= 3) {
            
            auto [_, disk_id, first_block] = tag_block.back();
            tag_block.pop_back();
            if(is_occupied[disk_id]) {
                continue ;
            }

            is_occupied[disk_id] = true;
            const auto& dk = disk[disk_id].disk_block_gu[first_block];
            auto [block_pos, tar] = get_continuos_unit(disk_id, dk.first_point, dk.first_point + dk.size - 1, size);

            do_write(id, size, xth, disk_id, block_pos);
            printf("\n");
            xth += 1;

            disk[disk_id].add_object_gu(id, size, tag, first_block);
        }

        if(xth > 3) {
            continue;
        }
        
        //  2.开辟固定块
        while(xth <= 3) {
            auto [disk_id, first_block]  = get_empty_block(tag, is_occupied);
            if(disk_id == 0) break;
            is_occupied[disk_id] = true;
            const auto& dk = disk[disk_id].disk_block_gu[first_block];
            auto [block_pos, tar] = get_continuos_unit(disk_id, dk.first_point, dk.first_point + dk.size - 1, size);

            do_write(id, size, xth, disk_id, block_pos);
            printf("\n");
            xth += 1;
            disk[disk_id].add_object_gu(id, size, tag, first_block);
        }

        if(xth > 3) {
            continue;
        }

        //  3.寻找随机块
        for(int i = 1;i <= N && xth <= 3;i++) {
            auto& dk = disk[i].disk_block_sui;
            if(is_occupied[i] == false && dk.empty_num >= size) {
                is_occupied[i] = true;
                auto [block_pos, tar] = get_continuos_unit(i, dk.first_point, dk.first_point + dk.size - 1, size);
                do_write(id, size, xth, i, block_pos);
                printf("\n");
                xth += 1;
                disk[i].add_object_sui(id, size);
            }
        }

        //  4.没办法了，调参吧.
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

        bool flag = false;
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


int main()
{

    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);
   
    for (int i = 1; i <= N; i++) {
        disk[i].set(V, G);
    }

    for (int i = 1; i <= 3 * M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d", &global_state[i][j]);
        }
    }

    put_ok();
    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        delete_action();
        write_action();
        read_action();
    }
    return 0;
}