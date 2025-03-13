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

extern int T, M, N, V, G;
// 存每1800时间片的全部操作的数量
extern int global_state[3 * MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];



void put_ok()
{
	printf("OK\n");
	fflush(stdout);
}


