#include <iostream>
#include "b_tree.h"
#include "b_tree_node.h"

int main()
{
    // auto a = BTree<int,int,4>();
    auto n = BTree<4>();
    n.put("mud","mud");
    n.put("rare","rare");
    n.put("plop","plop");
    n.put("kone","kone");
    n.put("beast","beast");
    n.put("lorax","lorax");
    n.put("alpha","alpha");
    n.put("corn", "corn");
    n.put("ca", "ca");
    n.put("cb", "cb");
    n.put("cc", "cc");
    std::cout << "Adding Rares" << std::endl;
    n.put("rares", "rares");
    n.put("noop", "noop");
    // n.put("rareu", "rareu");
    n.put("raret", "raret");
    // n.put("rarew", "rarew");
    std::cout << "Hello World" << std::endl;
    n.root->print();
    std::cout << "Starting tests... 4" << std::endl;
    std::cout << "End" << std::endl;
    return 0;
}
