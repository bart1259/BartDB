#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "b_tree.h"
#include "b_tree_node.h"
#include "fileio.h"
#include <iomanip>
#include "database.h"
#include "memory_storage_engine.h"


void test_serializability() {
    MemoryStorageEngine<7>* mem_engine = new MemoryStorageEngine<7>();
    auto n = new BTree<7>(mem_engine);
    n->put("A", "A");
    n->put("B", "B");
    n->put("C", "C");
    n->put("D", "D");
    n->put("E", "E");
    n->put("F", "F");
    n->put("G", "G");

    BTreeNode<7>* root = n->root;
    char* root_bytes = b_tree_node_to_bytes(root);
    BTreeNode<7>* new_root = bytes_to_b_tree_node<7>(n, root_bytes);

    assert(root->get_node_id() == new_root->get_node_id());
    assert(root->get_parent_id() == new_root->get_parent_id());
    assert(root->get_size() == new_root->get_size());

    for (size_t i = 0; i < root->get_size(); i++)
    {
        // std::cout << i << std::endl;
        // std::cout << root->get_key_at(i) << "==" << new_root->get_key_at(i)  << std::endl;
        assert(root->get_key_at(i) == new_root->get_key_at(i));
        // std::cout << root->get_value_at(i) << "==" << new_root->get_value_at(i)  << std::endl;
        assert(root->get_value_at(i) == new_root->get_value_at(i));
        assert(root->get_child_id_at(i) == new_root->get_child_id_at(i));
    }
    
}

int main() {
    test_serializability();
    std::cout << "All Tests Pass!" << std::endl;
}