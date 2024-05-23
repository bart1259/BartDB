#ifndef B_TREE_NODE_H
#define B_TREE_NODE_H

#include <array>
#include <memory>
#include <cstring>
#include "b_tree.h"

template <typename K, typename V, int M>
class BTree;

/**
 * The node of a B-Tree
 * K - Class type of key
 * V - Class tpye of values
 * M - Max capacity of the node
*/
template <typename K, typename V, int M>
class BTreeNode
{
    static_assert(M % 2 == 0, "For this implementation, the max keys in a B-Tree must be even");
private:
    BTree<K,V,M>* tree;
    int parent_id = -1;

    std::array<K,M> keys;
    std::array<V,M> values;
    std::array<int,M+1> children_ids;

    int size = 0;

    inline K get_median(std::array<K,M> values, K value) {
        K lower_value = values[(size/2) - 1];
        K upper_value = values[(size/2)];

        if (value < lower_value) {
            return lower_value;
        }
        if (value > upper_value) {
            return upper_value;
        }
        return value;
    }

public:
    BTreeNode(BTree<K,V,M>* tree){
        this->tree = tree;
        std::fill(children_ids.begin(), children_ids.end(), -1);
        static_assert(sizeof(BTreeNode<K,V,M>) < 4096);
    }
    ~BTreeNode(){

    }
    void put(K key, V value) {

        if (size == M) {
            // The node is full, we do not have space for it.
            K median_key = get_median(this->values, key);
            std::cout << "Meidan is " << median_key << std::endl;
            // Node is root node
            if (parent_id == -1) {
                std::cout << "Splitting root node" << std::endl;

                int left_node_id = tree->create_new_node();
                int right_node_id = tree->create_new_node();

                BTreeNode<K,V,M>* left_node = tree->get_node(left_node_id);
                BTreeNode<K,V,M>* right_node = tree->get_node(right_node_id);

                for(int i = 0; i < size; i++) {
                    // Add everything left of median key to left node
                    if (keys[i] < median_key) {
                        left_node->put(keys[i], values[i]);
                    }
                    // Add everything right of the median key to the right node
                    if (keys[i] > median_key) {
                        right_node->put(keys[i], values[i]);
                    }
                    // Set median to zeroth index
                    if (keys[i] == median_key) {
                        keys[0] = keys[i];
                        values[0] = values[i];
                    }
                }

                // Clear remainder of list
                for (int i = 1; i < M; i ++) {
                    keys[i] = 0;
                    values[i] = 0;
                    children_ids[i] = -1;
                }
                children_ids[M] = -1;

                std::cout << "Making two childern with IDs " << left_node_id << " + " << right_node_id << std::endl;

                children_ids[0] = left_node_id;
                children_ids[1] = right_node_id;

                size = 1;

            }
        } else {
            if (size == 0) {
                keys[0] = key;
                values[0] = value;
                size += 1;
                return;
            }
            // Iterate over every key and see where it goes
            bool inserted = false;
            for (int i = 0; i < size; i++) {
                if (key < keys[i]) {
                    // Shift entire list to the right
                    //FIXME: Also shift pointers
                    for (int j = size; j > i; j--)
                    {
                        keys[j] = keys[j-1];
                        values[j] = values[j-1];
                    }
                    keys[i] = key;
                    values[i] = value;
                    
                    inserted = true;
                    size += 1;
                    return;
                }
            }

            if (inserted == false) {
                // Add to end
                keys[size] = key;
                values[size] = value;
                size += 1;
                return;
            }
        }

        return;
    }

    void print() {
        std::cout << "| ";
        for (int i = 0; i < size; i++) 
            std::cout << keys[i] << " | ";
        for (int i = size; i < M; i++)
            std::cout << "   | ";

        std::cout << std::endl;
        for (int i = 0; i < M; i ++) {
            if (children_ids[i] != -1) {
                tree->get_node(children_ids[i])->print();
            }
        }
    }
};


#endif