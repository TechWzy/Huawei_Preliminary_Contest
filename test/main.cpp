#pragma once
#include"operation.hpp"

Unit unit[MAX_DISK_NUM][MAX_DISK_SIZE];
Request request[MAX_REQUEST_NUM];
Disk disk[MAX_DISK_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM][MAX_OBJECT_SIZE];

int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int is_activated[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int WriteIn[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int tag_allocation[MAX_TAG_NUM];
int tag_unit_number[MAX_TAG_NUM];

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

void writeIn_Position(int disk_id, const vector<int>& block_pos) {
    for(auto v : block_pos) {
        disk[disk_id].add_unit(v);
    }
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

        //  1.优先写入 large_block
        {
            int index = Is_LargeBlock_available(tag_allocation[tag], tag, size);
            if(index) {
                auto& dk = disk[tag_allocation[tag]];
                auto& bl = dk.block[index];
                auto [block_pos, tar] = get_continuos_unit(tag_allocation[tag], bl.first_point, bl.first_point + bl.size - 1, size);
                do_write(id, size, xth, tag_allocation[tag], block_pos);
                is_occupied[tag_allocation[tag]] = 1;
                printf("\n");
                xth += 1;
                writeIn_Position(tag_allocation[tag], block_pos);
            }
        }

        //  2.寻找可以写入的动态块
        auto avial_existed_block = get_existed_block(tag, is_occupied, size);
        std::sort(avial_existed_block.rbegin(), avial_existed_block.rend());

        while(xth <= 3 && !avial_existed_block.empty()) {
            
            auto [_, disk_id, index] = avial_existed_block.back();
            avial_existed_block.pop_back();

            auto& dk = disk[disk_id];
            auto& bl = dk.block[index];
            auto [block_pos, tar] = get_continuos_unit(disk_id, bl.first_point, bl.first_point + bl.size - 1, size);
            do_write(id, size, xth, disk_id, block_pos);
            is_occupied[disk_id] = 1;
            printf("\n");
            xth += 1;
            writeIn_Position(disk_id, block_pos);
        }

        if(xth > 3) continue;

        //  3.开辟动态块
        while(xth <= 3) {

            auto [disk_id, l, r] = get_empty_block(tag, size, is_occupied);
            if(!disk_id) break;
            
            int L = l, R = std::min(V, l + small_block_size - 1);
            //  分配标签块
            auto flag = disk[disk_id].allocate_block(L, R, tag);
            assert(flag == true);

            auto [block_pos, tar] = get_continuos_unit(disk_id, L, R, size);
            do_write(id, size, xth, disk_id, block_pos);
            is_occupied[disk_id] = 1;
            printf("\n");
            xth += 1;
            writeIn_Position(disk_id, block_pos);
        }

        if(xth > 3) continue;

        //  4. 写入相似度较高的变量中
        std::vector<std::tuple<double, int>> SimilarTag;
        
        for(int k = 1;k <= M;k++) {
            if(k != tag) {
                auto sim = Get_Similar_Level(k, tag, current_time());
                SimilarTag.push_back({sim, k});
            }
        }

        std::sort(SimilarTag.begin(), SimilarTag.end());

        while(xth <= 3 && !SimilarTag.empty()) {
            
            auto [_, match_tag] = SimilarTag.back();
            auto [empty_num, disk_id, index] = WriteIn_Other_Block(match_tag, size, is_occupied);
            
            if(empty_num == 0) {
                SimilarTag.pop_back();
                continue ;
            }

            auto& bl = disk[disk_id].block[index];
            auto [block_pos, tar] = get_continuos_unit(disk_id, bl.first_point, bl.first_point + bl.size - 1, size);
            do_write(id, size, xth, disk_id, block_pos);
            is_occupied[disk_id] = 1;
            printf("\n");
            xth += 1;
            writeIn_Position(disk_id, block_pos);
        } 

        //  5. 没办法了，调参吧
        if(xth <= 3) {
            file<<"current_time "<<current_time()<<endl;
            file.close();
        }
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

void init() {

    const int total = (T - 1) / FRE_PER_SLICING + 1;
    for(int i = 1;i <= M;i++) {
        for(int j = 1;j <= total;j++) {
            is_activated[i][j] = (Get_global_info(Read, i, j) >= activavte_threshold);
        }
    }

    for(int i = 1;i <= M;i++) {
        for(int j = total;j >= 1;j--) {
            WriteIn[i][j] = WriteIn[i][j + 1] + Get_global_info(Write, i, j);
        }
    }

    std::vector<std::array<int, 2>>Pre_allocation;
    for(int i = 1;i <= M;i++) {
        int need = ((int)(WriteIn[i][1] * large_block_init_ratio - 1) / 100 + 1) * 100;
        Pre_allocation.push_back({need, i});
    }

    for(int i = 1;i <= N;i++) {
        disk[i].set(V, i);
    }

    //  确保 need <= V， 否则需要重新制定策略.
    //  先不考虑标签间的冲突
    for(int i = 1, j = 1;i <= M;j = j % N + 1) {
        
        auto [need, tag] = Pre_allocation[i - 1];
        auto avail = disk[j].get_available_block();
        
        int L = 0, R = 0;
        for(auto [l, r] : avail) {
            if(r - l + 1 >= need) {
                L = l, R = l + need - 1;
                break;
            }
        }

        if(L) {
            disk[j].allocate_block(L, R, tag, true);
            tag_allocation[tag] = j;
            i += 1;
        }
    }
}

void update_block_action() {
    
    int xth = (current_time() - 1) / FRE_PER_SLICING + 1;
    for(int i = 1;i <= N;i++) {
        auto& dk = disk[i];
        auto infos = dk.infos;
        for(auto [l, r, index] : infos) {
            auto& bl = dk.block[index];
            const int res = (r - l + 1) - bl.empty_num;
            if(res <= delete_threshold && tag_unit_number[bl.tag] - bl.size >= WriteIn[bl.tag][xth + 1] && !bl.Is_Permanent) {
                file<<"current_time "<<current_time()<<endl;
                file<<"l r tag res "<<l<<" "<<r<<" "<<dk.block[index].tag<<" "<<res<<endl;
                dk.delete_block(index);
            } 
        }
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
        current_time(1);
        timestamp_action();
        if(t % 100 == 0) {  //  时间片间隔太短的情况下，删除情况不明显.
            update_block_action();
        }
        delete_action();
        write_action();
        read_action();
    }

    file.close();
    return 0;
}