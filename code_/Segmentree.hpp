#pragma once
#pragma GCC optimize("O2")
#include "utils.hpp"

/*
    线段是应该处理 请求上传、删除、过时等情况.

    base[i] : 记录一个节点的基准价值.
    query(l, r) 能过很好地估算该区间请求的价值总量.

*/

struct Segmentree{
    
    vector<int>ls, rs;
    vector<double>base, sum;
    vector<int>tag;
    
    Segmentree() {
        
    }

    Segmentree(int n): ls(n << 2), rs(n << 2), base(n << 2), sum(n << 2), tag(n << 2) {
        build(1, 1, n);
    }

    void build(int u, int l, int r) {
        ls[u] = l, rs[u] = r;
        if(l == r) {
            return ;
        }
        int mid = l + r >> 1;
        build(2 * u, l, mid);
        build(2 * u + 1, mid + 1, r);
    }

    void push_up(int u) {
        sum[u] = sum[2 * u] + sum[2 * u + 1];
        base[u] = base[2 * u] + base[2 * u + 1];
    }

    void push_down(int u) {
        if(tag[u]) {
            sum[2 * u] -= base[2 * u] * tag[u];
            sum[2 * u + 1] -= base[2 * u + 1] * tag[u];
            tag[2 * u] += tag[u];
            tag[2 * u + 1] += tag[u];
            tag[u] = 0;
        }
    }

    //  仅更新sum值.
    void update(int u, int l, int r, int c) {
        if(l <= ls[u] && rs[u] <= r) {
            sum[u] -= 1.0 * c * base[u];
            tag[u] += c;
            return ;
        }
        push_down(u);
        int mid = ls[u] + rs[u] >> 1;
        if(l <= mid) {
            update(2 * u, l, r, c);
        }
        if(r > mid) {
            update(2 * u + 1, l, r, c);
        }
        push_up(u);
    }

    //  处理请求上传、删除和过时等情况
    //  c 代表该请求的base值 、 res 代表该请求的剩余时间片
    void modify(int u, int x, const double c, const int res) {
        if(x <= ls[u] && rs[u] <= x){
            base[u] -= c;
            sum[u] -= 1.0 * res * c;
            if(eq(base[u])) {
                base[u] = 0;
                sum[u] = 0;
            }
            return ;
        }
        push_down(u);
        int mid = ls[u] + rs[u] >> 1;
        if(x <= mid) {
            modify(2 * u, x, c, res);
        } else {
            modify(2 * u + 1, x, c, res);
        }
        push_up(u);
    }

    void add(int u, int x, const double bs) {
        if(x <= ls[u] && rs[u] <= x) {
            base[u] += bs;
            sum[u] += bs * EXIST_TIME;
            return ;
        }
        push_down(u);
        int mid = ls[u] + rs[u] >> 1;
        if(x <= mid) {
            add(2 * u, x, bs);
        } else {
            add(2 * u + 1, x, bs);
        }
        push_up(u);
    } 

    double query(int u, int l, int r) {
        if(l <= ls[u] && rs[u] <= r) {
            return sum[u];
        }
        push_down(u);
        int mid = ls[u] + rs[u] >> 1;
        double res = 0;
        if(l <= mid) {
            res += query(2 * u, l, r);
        }
        if(r > mid) {
            res += query(2 * u + 1, l, r);
        }
        return res;
    }
};

