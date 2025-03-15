#include "utils.hpp"
#include "Block.hpp"
#include "Disk.hpp"
#include "Unit.hpp"
#include "Request.hpp"
#include "Object.hpp"


int T, M, N, V, G;
int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];

int timestamp;
Request request[MAX_REQUEST_NUM];
Object object[MAX_OBJECT_NUM];
Block block[MAX_OBJECT_NUM * (MAX_OBJECT_SIZE - 1) + 1];
Disk disk[MAX_DISK_NUM];
Unit unit[(MAX_DISK_NUM - 1) * (MAX_DISK_SIZE - 1) + 1];

void timestamp_action()
{
    timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);

    fflush(stdout);
}

void do_object_delete(const int* object_unit, int* disk_unit, int size)
{
    for (int i = 1; i <= size; i++) {
        disk_unit[object_unit[i]] = 0;
    }
}

void delete_action()
{
    int n_delete;
    int abort_num = 0;
    static int _id[MAX_OBJECT_NUM];

    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++) {
        scanf("%d", &_id[i]);
    }

    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0) {
            if (request[current_id].is_done == false) {
                abort_num++;
            }
            current_id = request[current_id].prev_id;
        }
    }

    printf("%d\n", abort_num);
    for (int i = 1; i <= n_delete; i++) {
        int id = _id[i];
        int current_id = object[id].last_request_point;
        while (current_id != 0) {
            if (request[current_id].is_done == false) {
                printf("%d\n", current_id);
            }
            current_id = request[current_id].prev_id;
        }
        for (int j = 1; j <= REP_NUM; j++) {
            do_object_delete(object[id].unit[j], disk[object[id].replica[j]], object[id].size);
        }
        object[id].is_delete = true;
    }

    fflush(stdout);
}


// д��λ�ò���
void do_write(int id,int size,int j,int dk_id,int first_empty) {
    printf("%d", dk_id);
    // Ѱ���ռ���������λ��
    int curcurrent_write_num = 0;
    std::vector<int> block_pos;
    for (int k = first_empty; k <= V; k++) {
        if (!unit[dk_id][k].is_exist) {
            // ���µ�Ԫ״̬
            printf(" %d", k);
            unit[dk_id][k].add_block(id, ++curcurrent_write_num);
            block_pos.push_back(k);
        }
        if (curcurrent_write_num == size) {

            // ��λ�ñ��浽������
            object[id].set_pos(j, block_pos, dk_id);
            break;
        }
    }
}


// ���һ����ǩ�Ƿ��й��̶���,���ҹ̶�����װ��
int check_tag_is_exixt(int tag, int size, int& first_empty, int& first_empty_block, bool is_have_copy[]) {
    for (int i = 1; i <= MAX_DISK_NUM - 1; i++) {

        // ͬһ������ĸ�����������һ������
        if (is_have_copy[i]) continue;
        auto [new_first_empty, new_first_empty_block] = disk[i].check_tag(tag, size);
        first_empty = new_first_empty;
        first_empty_block = new_first_empty_block;
        if (first_empty && first_empty_block) {
            return i;
        }
    }
    return 0;

//  object_unit[i] : 第 i 个对象块的存储位置
void do_object_write(int* object_unit, int* disk_unit, int size, int object_id)
{
    int current_write_point = 0;
    for (int i = 1; i <= V; i++) {
        if (disk_unit[i] == 0) {
            disk_unit[i] = object_id;
            object_unit[++current_write_point] = i;
            if (current_write_point == size) {
                break;
            }
        }
    }

    assert(current_write_point == size);

}

void write_action()
{
    int n_write;

    // ͬһ������ĸ������ܴ�ͬһ������
    bool is_have_copy[MAX_DISK_NUM] = { 0 };
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++) {
        int id, size;
        scanf("%d%d%*d", &id, &size);
        object[id].last_request_point = 0;
        for (int j = 1; j <= REP_NUM; j++) {
            object[id].replica[j] = (id + j) % N + 1;
            object[id].unit[j] = static_cast<int*>(malloc(sizeof(int) * (size + 1)));
            object[id].size = size;
            object[id].is_delete = false;
            do_object_write(object[id].unit[j], disk[object[id].replica[j]], size, id);
        }

        printf("%d\n", id);

        memset(is_have_copy, 0, sizeof(is_have_copy));

        // ǰ����������̶���
        for (int j = 1; j <= REP_NUM; j++) {


            int first_empty = 0, first_empty_block = 0;

            int dk_id = check_tag_is_exixt(tag, size,first_empty, first_empty_block, is_have_copy);

            // û�г��ֹ��ҵ�һ���տ�
            if (!dk_id) {
                dk_id = (id + j) % N + 1;
                // �ҵ����Բ���Ĵ���id
                auto [new_first_empty, new_first_empty_block] = disk[dk_id].disk_want_write_gu(tag, size);
                first_empty = new_first_empty;
                first_empty_block = new_first_empty_block;
                int temp_cnt = 1;
                // һֱ�ң����10��
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



            printf("%d", object[id].replica[j]);
            for (int k = 1; k <= size; k++) {
                printf(" %d", object[id].unit[j][k]);
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
                // ���´��̲�д��
                disk[dk_id].add_object_gu(id, size, tag, first_empty_block);
                do_write(id, size, j, dk_id, first_empty);
                printf("\n");
            }

        }

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

void read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++) {
        scanf("%d%d", &request_id, &object_id);
        request[request_id].object_id = object_id;
        request[request_id].prev_id = object[object_id].last_request_point;
        object[object_id].last_request_point = request_id;
        request[request_id].is_done = false;
    }


        // ��һ����飬�鿴���̻��Ƕ���
        int j;
        bool ok = false;
        for (j = 0; j < G; j++) {
            int temp_pos = disk[i].point + j;
            if (temp_pos > V) temp_pos -= V;
            if (check_value(i, temp_pos)) {
                ok = true;
                break;
            }

    static int current_request = 0;
    static int current_phase = 0;
    if (!current_request && n_read > 0) {
        current_request = request_id;
    }
    if (!current_request) {
        for (int i = 1; i <= N; i++) {
            printf("#\n");

        }
        printf("0\n");
    }
    else {
        current_phase++;
        object_id = request[current_request].object_id;
        for (int i = 1; i <= N; i++) {
            if (i == object[object_id].replica[1]) {
                if (current_phase % 2 == 1) {
                    printf("j %d\n", object[object_id].unit[1][current_phase / 2 + 1]);
                }
                else {
                    printf("r#\n");
                }
            }
            else {
                printf("#\n");
            }
        }

        if (current_phase == object[object_id].size * 2) {
            if (object[object_id].is_delete) {
                printf("0\n");
            }
            else {
                printf("1\n%d\n", current_request);
                request[current_request].is_done = true;
            }
            current_request = 0;
            current_phase = 0;
        }
        else {
            printf("0\n");
        }
    }

    fflush(stdout);
}

void clean()
{
    for (auto& obj : object) {
        for (int i = 1; i <= REP_NUM; i++) {
            if (obj.unit[i] == nullptr)
                continue;
            free(obj.unit[i]);
            obj.unit[i] = nullptr;
        }
    }
}


int main()
{

    // ��.in�ļ�������׼�����ض��򵽸��ļ�
    
    /*
    if (freopen("..//data//sample_practice.in", "r", stdin) == nullptr) {
        // ����ļ���ʧ�ܣ����������Ϣ������1
        perror("�޷����ļ�");
        return 1;
    }
    //*/
    


    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);
   
    for (int i = 1; i <= N; i++) {
        disk[i].set(V, G);
    }
    //��ȡȫ��״̬
    for (int i = 1; i <= 3 * M; i++) {

    scanf("%d%d%d%d%d", &T, &M, &N, &V, &G);

    for (int i = 1; i <= M; i++) {

        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    put_ok();

    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    for (int i = 1; i <= M; i++) {
        for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
            scanf("%*d");
        }
    }

    printf("OK\n");
    fflush(stdout);

    for (int i = 1; i <= N; i++) {
        disk_point[i] = 1;
    }


    for (int t = 1; t <= T + EXTRA_TIME; t++) {
        timestamp_action();
        delete_action();    
        write_action();     
        read_action();
    }
    clean();

    // �ر��ļ�������׼����ָ���Ĭ�ϵļ�������
    /*
    if (fclose(stdin) != 0) {
        perror("�޷��ر��ļ�");
        return 1;
    }
    //*/
    return 0;
}