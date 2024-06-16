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

#define INVALID_NODE_ID -1

// Although key_t and value_t is used more often, the c std lib already defines a key_t
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
        key_type median_key = get_median(this->values, k);
        std::cout << "  Splitting node. Key \"" << median_key << "\" has been selected for promotion" << std::endl;

        if (parent_id == INVALID_NODE_ID && is_leaf_node == true) {
            // Special rules if node is root
            std::cout << "Splitting root node with no children" << std::endl;

            int left_node_id = tree->create_new_node(node_id);
            int right_node_id = tree->create_new_node(node_id);

            std::cout << "Created new left (" << left_node_id << ") and right (" << right_node_id << ") root node IDs" << std::endl;

            BTreeNode<M>* left_node = tree->get_node(left_node_id);
            BTreeNode<M>* right_node = tree->get_node(right_node_id);

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
            if (k < median_key)
                left_node->put(k, v);
            if (k > median_key)
                right_node->put(k, v);
            if (k == median_key){
                keys[0] = k;
                values[0] = v;
            }

            // Clear remainder of list
            for (int i = 1; i < M; i ++) {
                keys[i] = key_type();
                values[i] = value_type();
                children_ids[i] = INVALID_NODE_ID;
            }
            children_ids[M] = INVALID_NODE_ID;

            children_ids[0] = left_node_id;
            children_ids[1] = right_node_id;

            is_leaf_node = false;
            size = 1;
        } else if (parent_id == INVALID_NODE_ID) {
            // Split root node with children
            std::cout << "Splitting root node with children" << std::endl;

            // In this case we promote the root median node to the root and split the children among two new nodes
            
            // Create two new children nodes
            int left_node_id = tree->create_new_node(node_id);
            int right_node_id = tree->create_new_node(node_id);

            std::cout << "Created new left (" << left_node_id << ") and right (" << right_node_id << ") node IDs as children of root." << std::endl;

            BTreeNode<M>* left_node = tree->get_node(left_node_id);
            BTreeNode<M>* right_node = tree->get_node(right_node_id);

            right_node->is_leaf_node = false;
            left_node->is_leaf_node = false;

            // Create comprehensive list of everything that needs to be added
            std::vector<kv_type> key_value_pairs = std::vector<kv_type>();
            std::vector<int> children = std::vector<int>();

            bool k_added = false;
            for (size_t i = 0; i < size; i++)
            {
                if (k < keys[i] && !k_added) {
                    key_value_pairs.push_back(kv_type(k, v));
                    k_added = true;
                }
                key_value_pairs.push_back(kv_type(keys[i], values[i]));
                children.push_back(children_ids[i]);
            }
            if(k_added == false) {
                key_value_pairs.push_back(kv_type(k, v));
            }
            children.push_back(children_ids[M]);

            std::cout << "  KV Pair size = " << key_value_pairs.size() << std::endl; 
            for (size_t i = 0; i < key_value_pairs.size(); i++)
            {
                std::cout << " | " << key_value_pairs[i].first;
            }
            std::cout << " |" << std::endl;
            std::cout << "  Children size = " << children.size() << std::endl;
            for (size_t i = 0; i < children.size(); i++)
            {
                std::cout << " | " << children[i];
            }
            std::cout << " |" << std::endl;

            assert(key_value_pairs.size() == M+1);
            assert(children.size() == M+1);

            // Add everything < median to left node
            for(int i = 0; i < M/2; i++) {
                // Add everything left of median key to left node
                left_node->keys[i] = key_value_pairs[i].first;
                left_node->values[i] = key_value_pairs[i].second;
                left_node->children_ids[i] = children[i];
            }
            left_node->children_ids[(M/2)] = children[M/2];
            left_node->size = M/2;

            // Add everything > median to right node
            for(int i = 0; i < M/2; i++) {
                // Add everything left of median key to left node
                right_node->keys[i] = key_value_pairs[(M/2)+i+1].first;
                right_node->values[i] = key_value_pairs[(M/2)+i+1].second;
                right_node->children_ids[i] = children[(M/2)+i+1];
            }
            // A void of pointers is created in the right node so it needs to filled
            int void_node_id = tree->create_new_node(right_node_id);
            right_node->children_ids[(M/2)] = void_node_id;
            right_node->size = M/2;

            // Update parent references
            // Iterate over left_node kids
            for (size_t i = 0; i < left_node->size+1; i++)
            {
                int child_id = left_node->children_ids[i];
                BTreeNode<M>* child = tree->get_node(child_id);
                child->set_parent_node_id(left_node_id);
            }
            // Iterate over right_node kids
            for (size_t i = 0; i < right_node->size+1; i++)
            {
                int child_id = right_node->children_ids[i];
                BTreeNode<M>* child = tree->get_node(child_id);
                child->set_parent_node_id(right_node_id);
            }
            
            //Update current node
            this->keys[0] = key_value_pairs[M/2].first;
            this->values[0] = key_value_pairs[M/2].second;
            this->children_ids[0] = left_node_id;
            this->children_ids[1] = right_node_id;
            this->size = 1;

        } else {
            std::cout << "(NODE_ID: " << node_id << ") Splitting non-root node " << std::endl;
            std::vector<kv_type> ltm;
            std::vector<kv_type> gtm;
            std::vector<int> ltc;
            std::vector<int> gtc;
            kv_type median;

            for (size_t i = 0; i < size; i++)
            {
                if (keys[i] < median_key) {
                    ltm.push_back(kv_type(keys[i], values[i]));
                    ltc.push_back(children_ids[i]);
                } else if (keys[i] > median_key) {
                    gtm.push_back(kv_type(keys[i], values[i]));
                    gtc.push_back(children_ids[i]);
                } else{
                    ltc.push_back(children_ids[i]);
                    median = kv_type(keys[i], values[i]);
                }
            }
            gtc.push_back(children_ids[M]);

            if (k < median_key) {
                ltm.push_back(kv_type(k, v));
            } else if (k > median_key) {
                gtm.push_back(kv_type(k, v));
            } else{
                median = kv_type(k, v);
            }

            this->clear();

            BTreeNode* parent = tree->get_node(parent_id);
            parent->handle_child_split(ltm, gtm, ltc, gtc, median, node_id);
            
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

    // This function is called by the child to a parent node when it is splitting.
    // @param ltm a list of nodes which are less than the median
    // @param gtm a list of nodes that are greater than the median
    // @param median the median node which is to be put into the current node
    // @param child_id the child node which is being split
    void handle_child_split(std::vector<kv_type> ltm, std::vector<kv_type> gtm, std::vector<int> ltc, std::vector<int> gtc, kv_type median, int child_id) {
        std::cout << "(NODE_ID: " << node_id << ") has been notified it's child (" << child_id << ") is splitting" << std::endl;
        if (size == M) {
            std::cout << "(NODE_ID: " << node_id << ") is full and cannot support child splitting. Splitting..." <<std::endl;

            this->split(median.first, median.second); // TODO: We might need to set a flag to not add the median going forward because it is getting promoted.
            std::cout << "(NODE_ID: " << node_id << ") split is complete. Median \"" << median.first << "\" was promoted" <<std::endl;
            if (parent_id == INVALID_NODE_ID) {
                // At this point, the median has been added somwhere but we still need to insert the LTM and GTM lists
                assert(size == 1);
                if (median.first == keys[0]) {
                    // Hard edge case, we need to split results on opposite ends of tree
                    assert(false);
                } else {
                    int insert_node_id = median.first < keys[0] ? children_ids[0] : children_ids[1];
                    BTreeNode<M>* insert_node = tree->get_node(insert_node_id);

                    // We've found the node where we need to add the ltm and gtm nodes to
                    // Find the exact values we need to shift
                    bool found = false;
                    for (size_t i = 0; i < insert_node->get_size(); i++) {
                        if (insert_node->get_key_at(i) == median.first) {
                            // Found. ltm to left of node, gtm to right of node
                            // Create new right node
                            int insert_left_id = tree->create_new_node(insert_node_id);
                            BTreeNode<M>* insert_left_node = tree->get_node(insert_left_id);
                            insert_node->children_ids[i] = insert_left_id;

                            int insert_right_id = insert_node->children_ids[i+1];
                            BTreeNode<M>* insert_right_node = tree->get_node(insert_right_id);

                            for (size_t j = 0; j < ltm.size(); j++)
                            {
                                insert_left_node->keys[j] = ltm[j].first;
                                insert_left_node->values[j] = ltm[j].second;
                                insert_left_node->children_ids[j] = ltc[j];
                            }
                            // Need to make another void node
                            int void_node_id = tree->create_new_node(insert_left_id);
                            insert_right_node->children_ids[ltm.size()] = void_node_id;   
                            insert_left_node->size = ltm.size();     
                            for (size_t j = ltm.size(); j <= M+1; j++) {
                                insert_left_node->children_ids[j] = INVALID_NODE_ID;
                            }          

                            for (size_t j = 0; j < gtm.size(); j++)
                            {
                                insert_right_node->keys[j] = gtm[j].first;
                                insert_right_node->values[j] = gtm[j].second;
                                insert_right_node->children_ids[j] = gtc[j];
                            }
                            insert_right_node->children_ids[gtm.size()] = gtc[gtm.size()];
                            insert_right_node->size = gtm.size();

                            for (size_t j = ltm.size(); j <= M+1; j++) {
                                insert_right_node->children_ids[j] = INVALID_NODE_ID;
                            }          
                            break;
                        }
                    }
                }


                // put(median.first, median.second);
                return;
            }
        }        

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
        
        keys[split_child_index-1] = median.first;
        values[split_child_index-1] = median.second;

        children_ids[split_child_index] =  new_node_id;
        size += 1;
    }

    void put(key_type key, value_type value) {
        if (!is_leaf_node) {
            // Find lower nodes to insert element into
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
            // If no subnode is valid then the value of the key > last key.
            //  so add it to the last b_tree child.
            if(!found_subnode) {
                tree->get_node(children_ids[size])->put(key, value);
            }
            return;
        }

        if (size == M) {
            // We must split the node
            split(key, value);
        } else {
            insert_into_array(key, value);
        }

        return;
    }

    // FIXME: make this only callable from inside the class
    void set_parent_node_id(int id) {
        this->parent_id = id;
    }

    int get_node_id() {
        return this->node_id;
    }

    key_type get_key_at(int index) {
        assert(index < size);
        return this->keys[index];
    }

    int get_child_id_at(int index) {
        assert(index <= size);
        return this->children_ids[index];
    }

    size_t get_size() {
        return this->size;
    }

    void print() {
        std::cout << node_id << " : (" << size << ")" << "| ";
        for (int i = 0; i < size; i++) 
            std::cout << keys[i] << " | ";
        for (int i = size; i < M; i++)
            std::cout << "- | ";
        std::cout << std::endl;
    

        // Print all children IDs below
        std::cout << "       ";
        for (int i = 0; i < size + 1; i++) {
            if (children_ids[i] != INVALID_NODE_ID) {
                std::cout << children_ids[i] << "   ";
            } else {
                break;
            }
        }
        std::cout << std::endl << std::endl;

        if(!is_leaf_node) {
            for (int i = 0; i < size+1; i ++) {
                if (children_ids[i] != INVALID_NODE_ID) {
                    tree->get_node(children_ids[i])->print();
                }
            }
        }
    }
};


#endif