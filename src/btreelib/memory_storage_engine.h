#ifndef MEMORY_STORAGE_ENGINE_H
#define MEMORY_STORAGE_ENGINE_H

#include "b_tree.h"
#include "abstract_storage_engine.h"

template <int M>
class MemoryStorageEngine : public AbstractStorageEngine<M>
{
private:
    int nodeCount = 0;
    std::unordered_map<node_id_t,BTreeNode<M>*> nodes;
    BTreeNode<M>* root = nullptr;
public:

    MemoryStorageEngine() {
        nodes = std::unordered_map<node_id_t,BTreeNode<M>*>();
    }

    node_id_t create_node() override {
        nodes[nodeCount] = new BTreeNode<M>(this->tree);
        nodes[nodeCount]->set_node_id(nodeCount);
        nodeCount++;
        return nodeCount-1;
    };

    BTreeNode<M>* get_node(node_id_t node_id) override {
        return nodes.at(node_id);
    };

    void save_node(BTreeNode<M> node) override {
        // No implementation in memory
    };

    BTreeNode<M>* get_root_node() override {
        if(root == nullptr) {
            root = new BTreeNode<M>(this->tree);
            root->set_parent_id(INVALID_NODE_ID);
            root->set_node_id(0);
            nodeCount += 1;
            nodes[0] = root;
        }
        return root;
    }
};

#endif