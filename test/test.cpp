#include <iostream>
#include <cstdlib>
#include "b_tree.h"
#include "b_tree_node.h"

#define MIN_KEY FixedString<64>("")
#define MAX_KEY FixedString<64>("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

template <int N>
int assert_order_node_recursively(BTree<N>* tree, BTreeNode<N>* node, key_type min, key_type max) {

    std::cout << "Checking node " << node->get_node_id() << " with " << node->get_size() << " keys. Expected min: \"" << min << "\" Expected max: \"" << max << "\"" << std::endl;

    int ret = 0;
    for (size_t i = 0; i < node->get_size(); i++)
    {
        assert(node->get_key_at(i) < max && node->get_key_at(i) > min);
        if (i != 0) {
            assert(node->get_key_at(i-1) < node->get_key_at(i));
        }
        if (i != node->get_size()-1) {
            assert(node->get_key_at(i) < node->get_key_at(i+1));
        }
        ret += 1;
    }

    // Traverse children
    key_type new_min = min;
    key_type new_max = min;
    for (size_t i = 0; i < node->get_size()+1; i++)
    {
        if (i != 0) {
            new_min = node->get_key_at(i-1);
        }

        if (i != node->get_size()) {
            new_max = node->get_key_at(i);
        } else {
            new_max = max;
        }

        int child_id = node->get_child_id_at(i);
        if (child_id != INVALID_NODE_ID) {
            BTreeNode<N>* child_node = tree->get_node(child_id);
            ret += assert_order_node_recursively(tree, child_node, new_min, new_max);
        }
    }

    return ret;
}

template <int N>
int assert_order(BTree<N>* tree) {
    // Assert the order of the tree is preserved

    // DO Tree traversal
    BTreeNode<N>* node = tree->root;
    return assert_order_node_recursively<N>(tree, node, MIN_KEY, MAX_KEY);
}


char* generate_random_string() {
    const int SIZE = 8;
    char* strg = (char*)malloc((SIZE + 1) * sizeof(char));
    for (size_t i = 0; i < SIZE; i++)
    {
        strg[i] = (char)(65 + (std::rand() % 26));
    }
    strg[SIZE] = '\0';
    return strg;
}

int main()
{
    std::srand(42);
    assert(((key_type)"Corn") < ((key_type)"ZZZZZZZZZZZZZ"));

    auto n = BTree<4>();
    for (size_t i = 0; i < 50; i++)
    {
        char* rand_string = generate_random_string();
        std::cout << " String inserted was: " << rand_string << std::endl;
        n.put(key_type(rand_string), key_type(rand_string));
        n.root->print();
        int nodes_traversed = assert_order(&n);
        assert(nodes_traversed == i+1);
    }
    


    // // auto a = BTree<int,int,4>();
    // auto n = BTree<4>();
    // n.put("mud","mud");
    // n.put("rare","rare");
    // n.put("plop","plop");
    // n.put("kone","kone");
    // n.put("beast","beast");
    // n.put("lorax","lorax");
    // n.put("alpha","alpha");
    // n.put("corn", "corn");
    // n.put("ca", "ca");
    // n.put("cb", "cb");
    // n.put("cc", "cc");
    // n.put("rares", "rares");
    // n.put("noop", "noop");
    // n.put("rareu", "rareu");
    // n.put("raret", "raret");
    // n.put("rarew", "rarew");
    // n.root->print();
    // n.put("rarez", "rarez");
    // n.root->print();

    // int nodes_traversed = assert_order(&n);
    // std::cout << "During test traversed " << nodes_traversed << " nodes" << std::endl;
    return 0;
}
