#include "netlemon/engine.h"

#include <iostream>
#include <asio.hpp>

int main()
{
    std::cout << "Hello world!" << netlemon::Sub(5, 10) << std::endl;
    std::cin.ignore();
    return 0;
}