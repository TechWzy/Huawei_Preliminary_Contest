#pragma once
#pragma GCC optimize("O2")
#include "utils.h"
/**

@brief 磁盘类
该类用于描述存储系统中的一个磁盘，它管理磁盘上存储单元的信息以及磁头的状态。
磁盘包含多个存储单元（块），同时维护以下属性：
@param id 磁盘的唯一标识符
@param empty_num 当前磁盘中空闲存储单元的数量
@param point 磁头当前所在的存储单元编号
@param object_num 磁盘中已存储的对象数量
@param object_id 当前磁盘中存储了那些对象
@param point_status 磁头状态，true 表示空闲，false 表示忙碌
@param rest_tokens 当前时间片中磁头剩余可使用的令牌数
@param first_empty 第一个空闲存储单元的位置
@param size 磁盘总的存储单元数

**/

class Disk {
public:
    int id;
    int empty_num;
    int point;
    int object_num;
    std::unordered_map<int, bool> object_id;
    int rest_tokens;
    int first_empty;
    int size;
};