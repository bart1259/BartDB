#ifndef B_TREE_H
#define B_TREE_H

#include <memory>
#include "b_tree_node.h"
#include "unordered_map"

/**
 * A B-Tree
 * K - Class type of key
 * V - Class tpye of values
 * M - Max capacity of the node
*/
template <typename K, typename V, int M>
class BTree
{
private:
    int nodeCount = 0;
    std::unordered_map<int,BTreeNode<K,V,M>*> nodes;
public:
    BTreeNode<K,V,M>* root;
    BTree() {
        //FIXME: at some point we will need to read this from a file
        this->root = new BTreeNode<K,V,M>(this);
        nodes[0] = this->root;
        nodeCount++;
    }
    
    ~BTree() {

    }

    int create_new_node() {
        nodes[nodeCount] = new BTreeNode<K,V,M>(this);
        nodeCount++;
        return nodeCount-1;
    }

    BTreeNode<K,V,M>* get_node(int node_id) {
        return nodes.at(node_id);
    }

    void put(K key, V value) {
        this->root->put(key, value);
    }
};


#endif