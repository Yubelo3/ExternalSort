#include <iostream>
#define BUFFER_SIZE 256
#include "ExternalSort.hpp"

int main()
{
    for (int i : {1, 2, 3, 4, 5, 6, 7})
    {
        std::cout << "=========================SORT " << i << "==============================" << std::endl;
        ExternalSort<int> sort(i);
        sort.excute();
        std::cout << "Buffer: " << BUFFER_SIZE << "*4 bytes" << std::endl;
        std::cout << "IO: " << sort.IOcount() << std::endl;
        std::cout << "elements: " << sort.num_elements() << std::endl;
        std::cout << "IO per elem: " << (float)sort.IOcount() * BUFFER_SIZE / sort.num_elements() << std::endl;
        std::cout << "=============================================================" << std::endl;
    }
    return 0;
}