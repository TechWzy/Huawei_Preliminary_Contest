#pragma once
#include "strategy.hpp"

Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
Request request[MAX_REQUEST_NUM];
Disk disk[MAX_DISK_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];
Segmentree seg[MAX_DISK_NUM];
vector<int> Outdated_Request[MAX_TIME];

int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int quan_test;

void timestamp_action()
{
    int timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);
    fflush(stdout);
}

void delete_action()
{
    int n_delete;
    int abort_num = 0;
    static int _id[MAX_OBJECT_NUM];
    std::set<int> abort_set; // 记录删除集合
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
        deleted_request(rd_id);
    }
    fflush(stdout);
}

void write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);
        object[id].set(size, tag);

        // 打印对象编号
        printf("%d\n", id);
        for (int j = 1; j <= REP_NUM; j++) {
            int dk_id = (id + j) % N + 1;

            // 找到可以插入的磁盘id
            int first_empty = disk[dk_id].add_object(id, size);
            while (!first_empty) {
                dk_id = (dk_id % N) + 1;
                first_empty = disk[dk_id].add_object(id, size);
            }
            printf("%d", dk_id);
            // 寻找收集具体插入的位置
            int curcurrent_write_num = 0;
            std::vector<int> block_pos;
            for (int k = first_empty; k <= V; k++) {
                if (!unit[dk_id][k].is_exist) {
                    // 更新单元状态
                    printf(" %d", k);
                    unit[dk_id][k].add_block(id, ++curcurrent_write_num);
                    block_pos.push_back(k);
                }
                if (curcurrent_write_num == size) {
                    // 将位置保存到对象中
                    object[id].set_pos(j, block_pos, dk_id);
                    // 更新第一个空位置
                    while (k <= V && !unit[dk_id][k].is_exist) k++;
                    disk[dk_id].first_empty = k;
                    break;
                }
            }
            printf("\n");
        }
    }

    fflush(stdout);
}

void read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++) {
        scanf("%d%d", &request_id, &object_id);
        // 添加并创建请求
        object[object_id].add_request(request_id);
        // 添加请求到达线段树
        add_request_to_segmentree(request_id);
    }
    int complete = 0;
    std::vector<int> complete_id;
    for (int i = 1; i <= N; i++) {

        // 第一波检查，查看是JUMP还是PASS
        int j;
        bool ok = false;
        for (j = 0; j <= G; j++) {
            if (check_value(i, disk[i].point + j)) {
                ok = true;
                break;
            }
        }

        // 该磁盘没机会读了
        if (ok == false) {
            // 检查该磁盘是否有有价值的块，如果有jump过去
            // 没有该磁盘略过
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
                // 每价值就不动
                printf("#\n");
                fflush(stdout);
            }
        }
        else { // 该磁盘有机会读，准备把tokens消耗完吧
            while (disk[i].rest_tokens > 0) {
                //  当贡献很少时，依旧选择 pass
                while (disk[i].rest_tokens > 0 && (!check_value(i, disk[i].point) || seg[i].query(1, disk[i].point, disk[i].point) < LIMIT)) {
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
        deleted_request(rq_id);
        fflush(stdout);
    }
}

int main()
{
    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);
    
    for (int i = 1; i <= N; i++) {
        disk[i].set(V, G);
    }

    for(int i = 1;i <= N;i++) {
        seg[i] = Segmentree(V);
    }

    //读取全局状态
    for (int i = 1; i <= 3 * M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%d", &global_state[i][j]);
        }
    }

    put_ok();
    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        current_time(1);
        timestamp_action();
        delete_action();
        write_action();
        read_action();
        for(auto rid : Outdated_Request[t]) {
            deleted_request(rid);
        }
        Outdated_Request[t].clear();
    }

    return 0;
}