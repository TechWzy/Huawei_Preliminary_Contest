#pragma once
#pragma GCC optimize("O2")
#include <bits/stdc++.h>
using namespace std;

using Sid = pair<set<int>, int>;
using un_set = unordered_set<int>;

const int MAX_DISK_NUM = 10 + 1;
const int MAX_DISK_SIZE = 16384 + 1;
const int MAX_REQUEST_NUM = 30000000 + 1;
const int MAX_OBJECT_NUM = 100000 + 1;
const int REP_NUM = 3;
const int FRE_PER_SLICING = 1800;
const int EXTRA_TIME = 105;
const int MAX_TAG_NUM = 16 + 1;
const int MAX_OBJECT_SIZE = 5 + 1;

int T, M, N, V, G;

//  记录全局预处理信息...
int global_del[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int global_write[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];
int global_read[MAX_TAG_NUM][MAX_REQUEST_NUM / FRE_PER_SLICING + 10];