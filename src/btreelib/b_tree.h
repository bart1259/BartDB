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
    BTreeNode<M>* root = nullptr;

    BTree(AbstractStorageEngine<M>* storage_engine) {
        this->storage_engine = storage_engine;
        this->storage_engine->set_tree(this);
    }
    
    ~BTree() {

    }

    int create_new_node() {
        return this->storage_engine->create_node();
    }

    BTreeNode<M>* get_node(int node_id) {
        return this->storage_engine->get_node(node_id);
    }

    void save_node(BTreeNode<M>* node) {
        this->storage_engine->save_node(node);
    }

    void put(key_type key, value_type value) {
        if(this->root == nullptr) {
            this->root = this->storage_engine->get_root_node();
        }
        this->root->put(key, value);
    }

    bool get(key_type key, value_type* value) {
        if(this->root == nullptr) {
            this->root = this->storage_engine->get_root_node();
        }
        return this->root->get(key, value);
    }
};


#endif