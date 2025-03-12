#pragma once
#pragma GCC optimize("O2")
#include "utils.h"

class Object {
public:
	int id;
	int size;
	int tag;
	int replica[REP_NUM];
};