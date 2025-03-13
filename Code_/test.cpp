#include"utils.h"
#include"Object.h"

void test_Object() {

    int id = 1, size = 2, tag = 1;

    Object obj(id, size, tag);
    obj.recieve(2);         //  读入编号为2的请求
    auto v = obj.Read(1);   //  读取对象块1

    for(auto u : v) {
        std::cout<<u<<" ";
    }
    std::cout<<"\n";

    obj.recieve(3);        //   读入编号为3的请求
    v = obj.Read(2);

    for(auto u : v) {
        std::cout<<u<<" ";
    }
    std::cout<<"\n";

    v = obj.clean();
    for(auto u : v) {
        std::cout<<u<<" ";
    }
    std::cout<<"\n";
}

int main(){
    
    test_Object();
    return 0;
}