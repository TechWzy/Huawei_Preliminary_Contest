#pragma once
#pragma GCC optimize("O2")
#include <bits/stdc++.h>

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
#define DISK_BLOCK_GU (9+1)
#define DISK_BLOCK_SUI (3+1)

#define next_step (5)

extern int T, M, N, V, G;
// ��ÿ1800ʱ��Ƭ��ȫ������������
extern int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];

int get_cost(int cur_cost) {
	return std::max(16, static_cast<int>(std::ceil(cur_cost * 0.8)));
}

void put_ok()
{
	printf("OK\n");
	fflush(stdout);
}


