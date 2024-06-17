#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "b_tree.h"
#include "b_tree_node.h"
#include "fileio.h"
#include <iomanip>
#include "database.h"
#include "memory_storage_engine.h"

#define DEBUG 0

#define MIN_KEY FixedString<64>("")
#define MAX_KEY FixedString<64>("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

template <int N>
int assert_order_node_recursively(BTree<N>* tree, BTreeNode<N>* node, key_type min, key_type max) {

    if (DEBUG)
        std::cout << "Checking node " << node->get_node_id() << " with " << node->get_size() << " keys. Expected min: \"" << min << "\" Expected max: \"" << max << "\"" << std::endl;

    int ret = 0;
    for (int i = 0; i < node->get_size(); i++)
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
    for (int i = 0; i < node->get_size()+1; i++)
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
    for (int i = 0; i < SIZE; i++)
    {
        strg[i] = (char)(65 + (std::rand() % 26));
    }
    strg[SIZE] = '\0';
    return strg;
}

//https://stackoverflow.com/questions/10599068/how-do-i-print-bytes-as-hexadecimal
#include <ctype.h>
#include <stdio.h>

void hexdump(void *ptr, int buflen) {
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    printf("%06x: ", i);
    for (j=0; j<16; j++) 
      if (i+j < buflen)
        printf("%02x ", buf[i+j]);
      else
        printf("   ");
    printf(" ");
    for (j=0; j<16; j++) 
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}

int main()
{
    MemoryStorageEngine<31>* mem_engine = new MemoryStorageEngine<31>();
    BTree<31>* n = new BTree<31>(mem_engine);

    std::srand(42);
    assert(((key_type)"Corn") < ((key_type)"ZZZZZZZZZZZZZ"));

    const int TEST_SIZE = 1000;
    std::vector<char*> random_strings; 
    for (int i = 0; i < TEST_SIZE; i++)
    {
        char* rand_string = generate_random_string();
        random_strings.push_back(rand_string);
    }

    // Insert in random order
    mem_engine = new MemoryStorageEngine<31>();
    n = new BTree<31>(mem_engine);
    for (int i = 0; i < random_strings.size(); i++)
    {
        if(DEBUG) {
            std::cout << "############ " << i << " ############" << std::endl;
            std::cout << " String inserted was: " << random_strings[i] << std::endl;
        }
        n->put(key_type(random_strings[i]), key_type(random_strings[i]));
        if(DEBUG) {
            n->root->print();
        }
        int nodes_traversed = assert_order(n);
        assert(nodes_traversed == i+1);
    }
    std::cout << "Random insertion test complete!" << std::endl;

    std::sort(random_strings.begin(), random_strings.end());
    
    // Insert in ascending order
    mem_engine = new MemoryStorageEngine<31>();
    n = new BTree<31>(mem_engine);
    for (int i = 0; i < random_strings.size(); i++)
    {
        if(DEBUG) {
            std::cout << "############ " << i << " ############" << std::endl;
            std::cout << " String inserted was: " << random_strings[i] << std::endl;
        }
        n->put(key_type(random_strings[i]), key_type(random_strings[i]));
        if(DEBUG) {
            n->root->print();
        }
        int nodes_traversed = assert_order(n);
        assert(nodes_traversed == i+1);
    }
    std::cout << "Ascending insertion test complete!" << std::endl;

    // Insert in descending order
    mem_engine = new MemoryStorageEngine<31>();
    n = new BTree<31>(mem_engine);
    int count = 0;
    for (int i = random_strings.size()-1; i >= 0; i--)
    {
        if(DEBUG) {
            std::cout << "############ " << count << " ############" << std::endl;
            std::cout << " String inserted was: " << random_strings[i] << std::endl;
        }
        n->put(key_type(random_strings[i]), key_type(random_strings[i]));
        if(DEBUG) {
            n->root->print();
        }
        int nodes_traversed = assert_order(n);
        assert(nodes_traversed == count+1);
        count += 1;
    }
    std::cout << "Descending insertion test complete!" << std::endl;
    return 0;
}
