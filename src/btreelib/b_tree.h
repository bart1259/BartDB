#ifndef B_TREE_H
#define B_TREE_H

#include <memory>
#include "b_tree_node.h"
#include "unordered_map"
#include "abstract_storage_engine.h"

/**
 * A B-Tree
 * M - Max capacity of the node
*/
template <int M>
class BTree
{
private:
    AbstractStorageEngine<M>* storage_engine;
public:
    BTreeNode<M>* root;

    BTree(AbstractStorageEngine<M>* storage_engine) {
        this->storage_engine = storage_engine;
        this->storage_engine->set_tree(this);
        this->root = this->storage_engine->get_root_node();
    }
    
    ~BTree() {

    }

    int create_new_node() {
        return this->storage_engine->create_node();
    }

    BTreeNode<M>* get_node(int node_id) {
        return this->storage_engine->get_node(node_id);
    }

    void put(key_type key, value_type value) {
        this->root->put(key, value);
    }
};


#endif