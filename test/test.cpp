#include <iostream>
#include "b_tree.h"
#include "b_tree_node.h"

int main()
{
    // auto a = BTree<int,int,4>();
    auto n = BTree<4>();
    n.put("4","4");
    n.put("8","8");
    n.put("7","7");
    n.put("2","2");
    n.put("1","1");
    std::cout << "Hello World" << std::endl;
    n.root->print();
    std::cout << "Starting tests... 4" << std::endl;
    std::cout << "End" << std::endl;
    return 0;
}
