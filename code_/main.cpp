#pragma once
#include "utils.hpp"
#include "Block.hpp"
#include "Disk.hpp"
#include "Unit.hpp"
#include "Request.hpp"
#include "Object.hpp"

Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
Request request[MAX_REQUEST_NUM];
Disk disk[MAX_DISK_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];

int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
std::queue<std::array<int, 2>>request_queue;

int read_global_state(Type type, int tag, int kth) {
    const int total = (T - 1) / FRE_PER_SLICING + 1;
    assert(kth >= 1 && kth <= total);
    assert(tag >= 1 && tag <= M);
    return global_state[tag + type * M][kth];
}

 std::ofstream file("tag_activation.txt");

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


void do_write(int id,int size,int j,int dk_id,int first_empty) {
    printf("%d", dk_id);

    int curcurrent_write_num = 0;
    std::vector<int> block_pos;
    for (int k = first_empty; k <= V; k++) {
        if (!unit[dk_id][k].is_exist) {
            printf(" %d", k);
            unit[dk_id][k].add_block(id, ++curcurrent_write_num);
            block_pos.push_back(k);
        }
        if (curcurrent_write_num == size) {
            object[id].set_pos(j, block_pos, dk_id);
            break;
        }
    }
}

int check_tag_is_lian(int dk_id, int pos, int size) {
    bool ok = true;
    for (int i = 0; i < size; i++) {
        int j = pos + i;
        if (j > V) j -= V;
        ok = ok && (!unit[dk_id][j].is_exist);
        if (!ok) {
            return false;
        }
    }
    return true;
}

//  判断是否存在磁盘已存储过 tag 对象.
int check_tag_is_exixt(int tag, int size, int& first_empty, int& first_empty_block, bool is_have_copy[]) {
    for (int i = 1; i <= MAX_DISK_NUM - 1; i++) {
        if (is_have_copy[i]) continue;
        std::vector<std::pair<int,int>> ans = disk[i].check_tag(tag, size);
        for (auto [new_first_empty, new_first_empty_block] : ans) {
            for (int j = new_first_empty; j < new_first_empty + disk[i].disk_block_gu[new_first_empty_block].size - size + 1; j++) {
                //  判断存储块是否连续...
                if (check_tag_is_lian(i, j, size)) {
                    first_empty = new_first_empty;
                    first_empty_block = new_first_empty_block;
                    return i;
                }
            }
        }
    }
    return 0;
}

void write_action()
{
    int n_write;
    bool is_have_copy[MAX_DISK_NUM] = { 0 };
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);
        object[id].set(size, tag);
        printf("%d\n", id);
        memset(is_have_copy, 0, sizeof(is_have_copy));
        for (int j = 1; j <= REP_NUM; j++) {
            int first_empty = 0, first_empty_block = 0;
            int dk_id = check_tag_is_exixt(tag, size,first_empty, first_empty_block, is_have_copy);
            if (!dk_id) {
                dk_id = (id + j) % N + 1;
                auto [new_first_empty, new_first_empty_block] = disk[dk_id].disk_want_write_gu(tag, size);
                first_empty = new_first_empty;
                first_empty_block = new_first_empty_block;
                int temp_cnt = 1;
                while (!first_empty_block || is_have_copy[dk_id]) {
                    dk_id = (dk_id % N) + 1;
                    temp_cnt++;
                    auto [new_first_empty, new_first_empty_block] = disk[dk_id].disk_want_write_gu(tag, size);
                    first_empty = new_first_empty;
                    first_empty_block = new_first_empty_block;
                    if (temp_cnt > MAX_DISK_NUM + 10) {
                        break;
                    }
                }
            }

            //  当找回到空的固定块 或 当前磁盘已经存储副本，那么直接存储到随即块.
            //  感觉随机块的读取效率自然是很低下的

            if (first_empty_block == 0 || is_have_copy[dk_id]) {
                dk_id = id % N + 1;
                int temp_cnt = 1;
                bool ok = false;
                first_empty = disk[dk_id].disk_want_write_sui(size);
        
                while (!ok) {
                    while (!first_empty || is_have_copy[dk_id]) {
                        dk_id = dk_id % N + 1;
                        temp_cnt++;
                        first_empty = disk[dk_id].disk_want_write_sui(size);
                        assert(temp_cnt < MAX_DISK_NUM + 10);
                    }
                    
                    
                    for (int k = first_empty; k < first_empty + disk[dk_id].disk_block_sui.size - size + 1; k++) {
                        if (check_tag_is_lian(dk_id, k, size)) {
                            first_empty = k;
                            ok = true;
                            break;
                        }
                    }

                    if (!ok) {
                        dk_id = dk_id % N + 1;
                        temp_cnt++;
                        first_empty = disk[dk_id].disk_want_write_sui(size);
                    }
                }

                disk[dk_id].add_object_sui(id, size);
                is_have_copy[dk_id] = true;
                do_write(id, size, j, dk_id, first_empty);
                printf("\n");
            }
            else {
                is_have_copy[dk_id] = true;
                disk[dk_id].add_object_gu(id, size, tag, first_empty_block);
                do_write(id, size, j, dk_id, first_empty);
                printf("\n");
            }
        }
    }

    fflush(stdout);
}

bool check_value(int dk_id , int pos)
{
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    return block[oj_id][bk_id].check();
}

bool predict(int dk_id) {

    int pt = disk[dk_id].point;
    int cost = disk[dk_id].last_take_tokens;
    int tot1 = 0, tot2 = 0;

    for(int i = 0;i < next_step;i++) {
        int cur_cost = get_cost(cost);
        tot1 += cur_cost;
        cost = cur_cost;
    }

    bool is_read = false;
    cost = disk[dk_id].last_take_tokens;
    for(int i = 0;i < next_step;i++, pt = pt % V + 1) {
        if(!check_value(dk_id, pt)) {
            tot2 += 1;
            is_read = false;
        } else if(is_read) {
            int cur_cost = get_cost(cost);
            tot2 += cur_cost;
            cost = cur_cost;
        } else {
            cost = 64;
            tot2 += cost;
            is_read = true;
        }
    }

    return tot1 <= tot2;
}

void delete_Outdated_request() {
    static int tot = 0;
    int cur_total = 0;
    int current_time = get_current_time();
    while(!request_queue.empty() && request_queue.front()[0] == current_time) {
        auto [_, rid] = request_queue.front();
        
        request_queue.pop();
        int oid = request[rid].object_id;
        //  当 oid == 0， 说明该对象已被删除了.
        if(oid) {
            //  file<<"rid oid size oid_tag "<<rid<<" "<<oid<<" "<<object[oid].size<<" "<<object[oid].tag<<"\n";
            cur_total += 1;
            tot += 1;
        }
        
        for(int i = 1;i <= object[oid].size;i++) {
            block[oid][i].delete_outdated_request(rid);
        }
    }
    if(cur_total) {
        //  file<<"current_time: "<<get_current_time()<<"\n";
        //  file<<"added_total: "<<cur_total<<" "<<" acc_total "<<tot<<"\n";
    }
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

    {
        const int total = (T - 1) / FRE_PER_SLICING + 1;
        const int limit = 800;
        for(int i = 1;i <= M;i++) {
            std::vector<int> is_activated(total + 1);
            for(int j = 1;j <= total;j++) {
                if(read_global_state(read, i, j) >= limit) {
                    is_activated[j] = 1;
                } 
            }
            file<<"tag: "<<i<<"\n";
            for(int j = 1;j <= total;j++) {
                file<<is_activated[j]<<" \n"[j == total];
            }
        }
    }
    
    put_ok();
    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        get_current_time(1);
        delete_Outdated_request();
        delete_action();
        write_action();
        read_action();
    }

    return 0;
}