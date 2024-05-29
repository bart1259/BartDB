#ifndef B_TREE_H
#define B_TREE_H

#include <memory>
#include "b_tree_node.h"
#include "unordered_map"

/**
 * A B-Tree
 * M - Max capacity of the node
*/
template <int M>
class BTree
{
private:
    int nodeCount = 0;
    std::unordered_map<int,BTreeNode<M>*> nodes;
public:
    BTreeNode<M>* root;
    BTree() {
        //FIXME: at some point we will need to read this from a file
        this->root = new BTreeNode<M>(this, 0, INVALID_NODE_ID);
        nodes[0] = this->root;
        nodeCount++;
    }
    
    ~BTree() {

    }

    int create_new_node(int parent_id) {
        nodes[nodeCount] = new BTreeNode<M>(this, nodeCount, parent_id);
        nodeCount++;
        return nodeCount-1;
    }

    BTreeNode<M>* get_node(int node_id) {
        return nodes.at(node_id);
    }

    void put(key_type key, value_type value) {
        this->root->put(key, value);
    }
};


#endif