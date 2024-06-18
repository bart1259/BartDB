
#ifndef ABSTRACT_STORAGE_ENGINE_H
#define ABSTRACT_STORAGE_ENGINE_H

#include "b_tree.h"

template <int M>
class AbstractStorageEngine
{
protected:
    BTree<M>* tree;
public:
    void set_tree(BTree<M>* tree) {
        this->tree = tree;
    }
    virtual node_id_t create_node() = 0;
    virtual BTreeNode<M>* get_node(node_id_t) = 0;
    virtual void save_node(BTreeNode<M>* node) = 0;
    virtual void flush_write() = 0;
    virtual BTreeNode<M>*  get_root_node() = 0;
};

#endif