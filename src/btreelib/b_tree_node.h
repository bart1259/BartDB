#ifndef B_TREE_NODE_H
#define B_TREE_NODE_H

#include <array>
#include <memory>
#include <cstring>
#include <cassert>
#include <vector>
#include "b_tree.h"
#include "fixed_string.h"

template <int M>
class BTree;

#define ROOT_NODE_ID 0
#define INVALID_NODE_ID -1

typedef FixedString<64> key_type;
typedef FixedString<64> value_type;
typedef std::pair<key_type, value_type> kv_type;

/**
 * The node of a B-Tree
 * K - Class type of key
 * V - Class tpye of values
 * M - Max capacity of the node
*/
template <int M>
class BTreeNode
{
    static_assert(M % 2 == 0, "For this implementation, the max keys in a B-Tree must be even");
private:
    BTree<M>* tree;
    int parent_id = INVALID_NODE_ID;
    int node_id = INVALID_NODE_ID;

    std::array<key_type,M> keys;
    std::array<value_type,M> values;
    std::array<int,M+1> children_ids;

    bool is_leaf_node = true;
    int size = 0;

    inline key_type get_median(std::array<key_type,M> values, key_type value) {
        key_type lower_value = values[(size/2) - 1];
        key_type upper_value = values[(size/2)];

        if (value < lower_value) {
            return lower_value;
        }
        if (value > upper_value) {
            return upper_value;
        }
        return value;
    }

    void clear() {
        std::fill(children_ids.begin(), children_ids.end(), INVALID_NODE_ID);
        size = 0;
    }

    void shift_right(int index, bool shift_left_pointer) {
        std::cout << index << std::endl;
        for (int j = size; j > index; j--) {
            std::cout << j << std::endl;
            keys[j] = keys[j-1];
            values[j] = values[j-1];
            children_ids[j+1] = children_ids[j];
        }
        if (shift_left_pointer) {
            children_ids[index+1] = children_ids[index];
        }
    }

    void insert_into_array(key_type k, value_type v) {
        assert(size != M);

        // Iterate over every key and see where it goes (Insertion sort)
        bool inserted = false;
        for (int i = 0; i < size; i++) {
            if (k < keys[i]) {
                shift_right(i, true); // FIXME: Think harder of true or false
                keys[i] = k;
                values[i] = v;
                
                inserted = true;
                size += 1;
                return;
            }
        }

        if (inserted == false) {
            // Add to end
            keys[size] = k;
            values[size] = v;
            size += 1;
            return;
        }
    }

    void split(key_type k, value_type v) {
        // Special rules if node is root
        if (parent_id == INVALID_NODE_ID) {
            // Root
        } else {
            key_type median_key = get_median(keys, k);
            std::vector<kv_type> ltm;
            std::vector<kv_type> gtm;
            kv_type median;

            for (size_t i = 0; i < size; i++)
            {
                if (keys[i] < median_key) {
                    ltm.push_back(kv_type(keys[i], values[i]));
                } else if (keys[i] > median_key) {
                    gtm.push_back(kv_type(keys[i], values[i]));
                } else{
                    median = kv_type(keys[i], values[i]);
                }
            }

            if (k < median_key) {
                ltm.push_back(kv_type(k, v));
            } else if (k > median_key) {
                gtm.push_back(kv_type(k, v));
            } else{
                median = kv_type(k, v);
            }

            this->clear();

            BTreeNode* parent = tree->get_node(parent_id);
            parent->handle_child_split(ltm, gtm, median, node_id);
            
        }
    }

public:
    BTreeNode(BTree<M>* tree, int node_id, int parent_id){
        this->tree = tree;
        this->node_id = node_id;
        this->parent_id = parent_id;
        std::fill(children_ids.begin(), children_ids.end(), INVALID_NODE_ID);
        static_assert(sizeof(BTreeNode<M>) < 4096);
    }
    ~BTreeNode(){

    }

    void handle_child_split(std::vector<kv_type> ltm, std::vector<kv_type> gtm, kv_type median, int child_id) {
        //FIXME: Perform check to see if full

        // Find where child_id is
        int split_child_index = -1;
        for (size_t i = 0; i < size+1; i++)
        {
            if (child_id == children_ids[i]) {
                split_child_index = i+1;
            }
        }

        assert(split_child_index != -1);

        // Shift entire array after split_child_index
        shift_right(split_child_index-1, true); // FIXME: Think harder of true or false

        // Make new node
        int new_node_id = tree->create_new_node(node_id);
        BTreeNode* new_node = tree->get_node(new_node_id);
        BTreeNode* original_child_node = tree->get_node(child_id);
        
        for(auto it = ltm.begin(); it != ltm.end(); it++) {
            original_child_node->put((*it).first, (*it).second);  
        }
        for(auto it = gtm.begin(); it != gtm.end(); it++) {
            new_node->put((*it).first, (*it).second);   
        }
        
        std::cout << split_child_index << " " << median.first << " " << median.second << std::endl;
        keys[split_child_index-1] = median.first;
        values[split_child_index-1] = median.second;

        children_ids[split_child_index] =  new_node_id;
        size += 1;
    }

    void put(key_type key, value_type value) {
        if (!is_leaf_node) {
            bool found_subnode = false;
            // Try lower nodes
            for (int i = 0; i < size; i++) {
                if (key < keys[i]) {
                    // Insert into node
                    tree->get_node(children_ids[i])->put(key, value);
                    found_subnode = true;
                    return;
                }
            } 
            if(!found_subnode) {
                tree->get_node(children_ids[size])->put(key, value);
            }
            return;
        }

        if (size == M) {
            // The node is full, we do not have space for it.
            key_type median_key = get_median(this->values, key);
            std::cout << "Meidan is " << median_key << std::endl;
            // Node is root node
            if (parent_id == INVALID_NODE_ID) {
                std::cout << "Splitting root node" << std::endl;

                std::cout << "Creating new nodes" << std::endl;
                int left_node_id = tree->create_new_node(node_id);
                int right_node_id = tree->create_new_node(node_id);

                std::cout << "Getting new nodes" << std::endl;
                BTreeNode<M>* left_node = tree->get_node(left_node_id);
                BTreeNode<M>* right_node = tree->get_node(right_node_id);

                std::cout << "Populating new nodes" << std::endl;
                for(int i = 0; i < size; i++) {
                    // Add everything left of median key to left node
                    if (keys[i] < median_key) {
                        std::cout << "Adding to left node " << keys[i] << std::endl;
                        left_node->put(keys[i], values[i]);
                    }
                    // Add everything right of the median key to the right node
                    if (keys[i] > median_key) {
                        std::cout << "Adding to right node " << keys[i] << std::endl;
                        right_node->put(keys[i], values[i]);
                    }
                    // Set median to zeroth index
                    if (keys[i] == median_key) {
                        keys[0] = keys[i];
                        values[0] = values[i];
                    }
                }
                // TODO: We could just call put instead of doing this.
                if (key < median_key)
                    left_node->put(key, value);
                if (key > median_key)
                    right_node->put(key, value);
                if (key == median_key)
                    values[0] = value;

                std::cout << "Clearing parent node" << std::endl;
                // Clear remainder of list
                for (int i = 1; i < M; i ++) {
                    keys[i] = key_type();
                    values[i] = value_type();
                    children_ids[i] = INVALID_NODE_ID;
                }
                children_ids[M] = INVALID_NODE_ID;

                std::cout << "Reassigning childern IDs " << left_node_id << " + " << right_node_id << std::endl;

                children_ids[0] = left_node_id;
                children_ids[1] = right_node_id;

                is_leaf_node = false;
                size = 1;

            } else {
                std::cout << "Splitting non-root node" << std::endl;
                split(key, value);
            }
        } else {
            insert_into_array(key, value);
        }

        return;
    }

    void print() {
        std::cout << node_id << " : (" << size << ")" << "| ";
        for (int i = 0; i < size; i++) 
            std::cout << keys[i] << " | ";
        for (int i = size; i < M; i++)
            std::cout << "  | ";
        std::cout << std::endl;
    

        // Print all children IDs below
        std::cout << "    ";
        for (int i = 0; i < size + 1; i++) {
            if (children_ids[i] != INVALID_NODE_ID) {
                std::cout << children_ids[i] << "   ";
            } else {
                break;
            }
        }
        std::cout << std::endl << std::endl;

        for (int i = 0; i < M+1; i ++) {
            if (children_ids[i] != INVALID_NODE_ID) {
                tree->get_node(children_ids[i])->print();
            }
        }
    }
};


#endif