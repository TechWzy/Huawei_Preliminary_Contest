#pragma once
#pragma GCC optimize("O2")
#include <bits/stdc++.h>
using namespace std;
using info = std::array<int, 3>;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)
#define MAX_TAG_NUM (16 + 1)
#define MAX_OBJECT_SIZE (5 + 1)
#define READ_TAKE_TOKENS (64)

#define DISK_BLOCK_NUM (12+1)
#define DISK_BLOCK_GU (9)
#define DISK_BLOCK_SUI (3+1)

#define next_step (9)

#define large_block_init_ratio (0.7)
#define small_block_size (200)
#define delete_threshold (30)
#define activavte_threshold (10000)

extern int T, M, N, V, G;
extern int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
extern int is_activated[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];

enum Type { Delete, Write, Read };

/*
	获取两个标签在 xth 时刻过后的冲突程度.
*/

double Get_Conflict_Level(int x, int y, int xth) {

	if(x == y) {
		return 0.0;
	}

	const int total = (T - 1) / FRE_PER_SLICING + 1;
	double sum = 0.0, v = 0.0;

	for(int i = xth;i <= total;i++) {
		sum += 1;
		v += (is_activated[x][i] & is_activated[y][i]);
	}

	return (v / sum);
} 

int Get_global_info(Type op, int tag, int xth) {
	const int total = (T - 1) / FRE_PER_SLICING + 1;
	assert(xth  >= 1 && xth <= total);
	return global_state[op * M + tag][xth];
}

int get_cost(int cur_cost) {
	return std::max(16, static_cast<int>(std::ceil(cur_cost * 0.8)));
}

int get_next_position(int cur, int st, int ed) {
	assert(st <= ed);
	return cur < ed?cur + 1 : st;
}

int current_time(int op = 0) {
	static int now_time = 0;
	now_time += op;
	return now_time;
}

void put_ok()
{
	printf("OK\n");
	fflush(stdout);
}