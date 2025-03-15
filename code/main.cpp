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
    }
    fflush(stdout);
}


// 写入位置操作
void do_write(int id,int size,int j,int dk_id,int first_empty) {
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
            break;
        }
    }
}


// 检查一个标签是否有过固定块,并且固定块能装下
int check_tag_is_exixt(int tag, int size, int& first_empty, int& first_empty_block, bool is_have_copy[]) {
    for (int i = 1; i <= MAX_DISK_NUM - 1; i++) {

        // 同一个对象的副本不能再在一个磁盘
        if (is_have_copy[i]) continue;
        auto [new_first_empty, new_first_empty_block] = disk[i].check_tag(tag, size);
        first_empty = new_first_empty;
        first_empty_block = new_first_empty_block;
        if (first_empty && first_empty_block) {
            return i;
        }
    }
    return 0;
}

void write_action()
{
    int n_write;

    // 同一个对象的副本不能存同一个磁盘
    bool is_have_copy[MAX_DISK_NUM] = { 0 };
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);
        object[id].set(size, tag);

        // 打印对象编号
        printf("%d\n", id);

        memset(is_have_copy, 0, sizeof(is_have_copy));

        // 前两个副本存固定块
        for (int j = 1; j <= REP_NUM - 1; j++) {

            int first_empty = 0, first_empty_block = 0;

            int dk_id = check_tag_is_exixt(tag, size,first_empty, first_empty_block, is_have_copy);

            // 没有出现过找第一个空块
            if (!dk_id) {
                dk_id = (id + j) % N + 1;
                // 找到可以插入的磁盘id
                auto [new_first_empty, new_first_empty_block] = disk[dk_id].disk_want_write_gu(tag, size);
                first_empty = new_first_empty;
                first_empty_block = new_first_empty_block;
                int temp_cnt = 1;
                // 一直找，最多10次
                while (!first_empty_block || is_have_copy[dk_id]) {
                    dk_id = (dk_id % N) + 1;
                    temp_cnt++;
                    auto [new_first_empty, new_first_empty_block] = disk[dk_id].disk_want_write_gu(tag, size);
                    first_empty = new_first_empty;
                    first_empty_block = new_first_empty_block;
                    if (temp_cnt > MAX_DISK_NUM + 10) {
                        break;
                    }
                    //assert(temp_cnt < MAX_DISK_NUM + 10);
                }


            }
            if (first_empty_block == 0 || is_have_copy[dk_id]) {
                dk_id = id % N + 1;
                int temp_cnt = 1;
                first_empty = disk[dk_id].disk_want_write_sui(size);
                while (!first_empty || is_have_copy[dk_id]) {
                    dk_id = dk_id % N + 1;
                    temp_cnt++;
                    first_empty = disk[dk_id].disk_want_write_sui(size);
                    assert(temp_cnt < MAX_DISK_NUM + 10);
                }
                disk[dk_id].add_object_sui(id, size);
                is_have_copy[dk_id] = true;
                do_write(id, size, j, dk_id, first_empty);
                printf("\n");
            }
            else {
                is_have_copy[dk_id] = true;
                // 更新磁盘并写入
                disk[dk_id].add_object_gu(id, size, tag, first_empty_block);
                do_write(id, size, j, dk_id, first_empty);
                printf("\n");
            }

        }

        // 是否有副本块放到随机区域
        for (int j = 3; j <= REP_NUM; j++) {
            int dk_id = id % N + 1;
            int temp_cnt = 1;
            int first_empty = disk[dk_id].disk_want_write_sui(size);
            while (!first_empty || is_have_copy[dk_id]) {
                dk_id = dk_id % N + 1;
                temp_cnt++;
                first_empty = disk[dk_id].disk_want_write_sui(size);
                assert(temp_cnt < MAX_DISK_NUM + 10);
            }
            disk[dk_id].add_object_sui(id, size);
            is_have_copy[dk_id] = true;
            do_write(id, size, j, dk_id, first_empty);
            printf("\n");
        }
    }

    fflush(stdout);
}

// 检查磁盘位置是否有价值
bool check_value(int dk_id , int pos)
{
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    return block[oj_id][bk_id].check();
}

std::vector<int> do_read(int dk_id) {
    std::vector<int> complete_id;
    int pos = disk[dk_id].point;
    int oj_id = unit[dk_id][pos].object_id;
    int bk_id = unit[dk_id][pos].block_order;
    std::set<int> temp_set = block[oj_id][bk_id].requested_id_block;
    for (int rq_id : temp_set) {

        // 检查可能完成的请求每一次
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

        // 添加并创建请求
        object[object_id].add_request(request_id);
    }
    int complete = 0;
    std::vector<int> complete_id;
    for (int i = 1; i <= N; i++) {

        // 第一波检查，查看读固还是读随
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
                while (disk[i].rest_tokens > 0 && !check_value(i, disk[i].point)) {
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
    // 打开.in文件并将标准输入重定向到该文件
    
    /*
    if (freopen("..//data//sample_practice.in", "r", stdin) == nullptr) {
        // 如果文件打开失败，输出错误信息并返回1
        perror("无法打开文件");
        return 1;
    }
    //*/
    


    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);
   
    for (int i = 1; i <= N; i++) {
        disk[i].set(V, G);
    }
    //读取全局状态
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

    // 关闭文件并将标准输入恢复到默认的键盘输入
    /*
    if (fclose(stdin) != 0) {
        perror("无法关闭文件");
        return 1;
    }
    //*/
    return 0;
}