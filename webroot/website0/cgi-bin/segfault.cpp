#include <iostream>
int main(void)
{
    int jan[3];
    jan[0] = 1;
    jan[1] = 2;
    jan[2] = 3;
    jan[3] = 4;
    std::cout << jan[0] << jan[1] << jan[2] << jan[-1] << std::endl;
    return 0;
}