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
    root->set_parent_id(17);
    root->set_node_id(201);
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

char* generate_random_string() {
    const int SIZE = 8;
    char* strg = (char*)malloc((SIZE + 1) * sizeof(char));
    for (int i = 0; i < SIZE; i++)
    {
        strg[i] = (char)(65 + (std::rand() % 26));
    }
    strg[SIZE] = '\0';
    return strg;
}

int main() {
    test_serializability();


    Database<31>* db = new Database<31>("test1");
    BTree<31>* n = new BTree<31>(db);
    db->open();

    const int TEST_SIZE = 1000;
    std::vector<char*> random_strings; 
    for (int i = 0; i < TEST_SIZE; i++)
    {
        char* rand_string = generate_random_string();
        random_strings.push_back(rand_string);
    }

    // Insert in random order
    for (int i = 0; i < random_strings.size(); i++)
    {
        std::cout << "############ " << i << " ############" << std::endl;
        std::cout << " String inserted was: " << random_strings[i] << std::endl;
        n->put(key_type(random_strings[i]), key_type(random_strings[i]));
    }
    std::cout << "Random insertion test complete!" << std::endl;

    std::cout << "All Tests Pass!" << std::endl;
}