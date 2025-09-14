#include <iostream>
#include "add.h"
#include "sub.h"

int main(){
    int a=myadd(5,6);
    std::cout<<"a="<<a<<std::endl;
    int b=mysub(9,6);
    std::cout<<"b="<<b<<std::endl;

    return 0;
}