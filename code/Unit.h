#pragma once
#pragma GCC optimize("O2")
#include "utils.h"

/**

@brief 存储单元（Unit）类
该类用于表示硬盘中的一个存储单元。一个硬盘可能包含成千上万个存储单元，
每个存储单元对应硬盘上的一个块。
示例：
假设硬盘上有两个对象，其中对象1由3个存储单元构成，对象2由4个存储单元构成，
则属性示例如下：
 对象1(3块)        对象2(4块)
 ------------------------------
object_id:   1   2   3     4   5   6   7
block_order: 1   2   3     1   2   3   4
@param disk_id 存储单元所在的磁盘编号
@param block_id 存储单元中存储的对象编号
@param object_id 对象块的标识ID（单调递增）
@param id 存储单元在磁盘中的具体位置
@param is_exist 标志存储单元是否被占用

*/

class Unit {
public:
	int disk_id; 
	int block_id; 
	int object_id; 
	int block_order; 
	int id; 
	bool is_exist; 

};