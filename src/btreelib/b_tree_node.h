#ifndef B_TREE_NODE_H
#define B_TREE_NODE_H

#include <array>
#include <memory>
#include <cstring>
#include <cassert>
#include <vector>
#include "b_tree.h"
#include "fixed_string.h"
#include <iostream>

template <int M>
class BTree;

#define VERBOSE_SPLITTING 0

#define INVALID_NODE_ID -1
#define NODE_ID "(NODE_ID: " << node_id << ")"

// Although key_t and value_t is used more often, the c std lib already defines a key_t
typedef FixedString<64> key_type;
typedef FixedString<64> value_type;
typedef std::pair<key_type, value_type> kv_type;
typedef short node_id_t;

/**
 * The node of a B-Tree
 * K - Class type of key
 * V - Class tpye of values
 * M - Max capacity of the node
*/
template <int M>
class BTreeNode
{
    static_assert(M % 2 == 1, "For this implementation, the max keys in a B-Tree must be odd");
private:
    node_id_t node_id = INVALID_NODE_ID;
    node_id_t parent_id = INVALID_NODE_ID;
    int size = 0;

    std::array<node_id_t,M+1> children_ids;
    std::array<key_type,M> keys;
    std::array<value_type,M> values;

    // bool is_leaf_node = true;
    BTree<M>* tree;

public:
    BTreeNode(BTree<M>* tree){
        assert(tree != nullptr);
        this->tree = tree;
        std::fill(children_ids.begin(), children_ids.end(), INVALID_NODE_ID);
        static_assert(sizeof(BTreeNode<M>) < 4096);
    }
    ~BTreeNode(){

    }

    void clear() {

    }

    void insert_into_leaf(key_type new_key, value_type new_value) {

        // First check if element already exists
        for ( int i = 0; i < size; i++) {
            if(new_key == keys[i]) {
                values[i] = new_value;
                return;
            }
        }


        // Find best spot to put the new element

        bool inserted = false;
        for (int i = size; i >= 0; i--)
        {
            if (keys[i-1] < new_key) {
                keys[i] = new_key;
                values[i] = new_value;
                inserted = true;
                break;
            }

            keys[i] = keys[i-1];
            values[i] = values[i-1];
        }

        if(inserted == false) {
            keys[0] = new_key;
            values[0] = new_value;
        }
        

        size += 1;
        tree->save_node(this);

        tree->flush_write();
    }

    bool is_leaf_node() {
        return children_ids[0] == INVALID_NODE_ID;
    }

    void split() {
        assert(size == M); // Should only be called when node is at capcaity.

        if (parent_id == INVALID_NODE_ID) {
            if(VERBOSE_SPLITTING) {
                std::cout << NODE_ID << ": Splitting root node" << std::endl;
            }

            // Root node
            
            node_id_t left_node_id = tree->create_new_node();
            node_id_t right_node_id = tree->create_new_node();

            BTreeNode<M>* left_node = tree->get_node(left_node_id);
            left_node->set_parent_id(node_id);
            BTreeNode<M>* right_node = tree->get_node(right_node_id);
            right_node->set_parent_id(node_id);

            left_node->size = M/2;
            right_node->size = M/2;

            // Copy left nodes to child
            for (int i = 0; i < M/2; i++)
            {
                left_node->keys[i] = keys[i];
                left_node->values[i] = values[i];
                left_node->children_ids[i] = children_ids[i];

                right_node->keys[i] = keys[i+(M/2)+1];
                right_node->values[i] = values[i+(M/2)+1];
                right_node->children_ids[i] = children_ids[i+(M/2)+1];
            }
            left_node->children_ids[M/2] = children_ids[M/2];
            right_node-> children_ids[M/2] = children_ids[M];

            tree->save_node(left_node);
            tree->save_node(right_node);
            

            // Update children to point to new parents
            for (int i = 0; i < left_node->size+1; i++)
            {
                if(left_node->is_leaf_node() == false) {
                    assert(left_node->children_ids[i] != 0);
                    BTreeNode<M>* child_node = tree->get_node(left_node->children_ids[i]);
                    child_node->parent_id = left_node->node_id;
                    tree->save_node(child_node);
                }
            }
            for (int i = 0; i < right_node->size+1; i++)
            {
                if(right_node->is_leaf_node() == false) {
                    assert(right_node->children_ids[i] != 0);
                    BTreeNode<M>* child_node = tree->get_node(right_node->children_ids[i]);
                    child_node->parent_id = right_node->node_id;
                    tree->save_node(child_node);
                }
            }
            
            // Update Self
            for (int i = 0; i < M+1; i++)
            {
                children_ids[i] = INVALID_NODE_ID;
            }
            this->size = 1;
            // this->is_leaf_node = false;
            this->keys[0] = this->keys[(M/2)];
            this->values[0] = this->values[(M/2)];
            this->children_ids[0] = left_node_id;
            this->children_ids[1] = right_node_id; 
            tree->save_node(this);

            tree->flush_write();
            
        } else {
            // Non root node

            // Promote median to parent node
            kv_type median = kv_type(keys[M/2], values[M/2]);
            if (VERBOSE_SPLITTING) {
                std::cout << NODE_ID << ": Splitting non-root node" << std::endl;
                std::cout << NODE_ID << ": Promoting median value: " << median.first << std::endl;
            }

            BTreeNode<M>* parent_node = tree->get_node(parent_id);
            assert(parent_node->size != M);

            // Create new greater than node
            node_id_t right_node_id = tree->create_new_node();
            BTreeNode<M>* right_node =  tree->get_node(right_node_id);
            right_node->set_parent_id(parent_id);

            // Find index in parent where median would fit.
            bool inserted = false;
            for (int i = parent_node->size; i >= 1; i--)
            {
                assert(i-1 >= 0);
                if (parent_node->keys[i-1] < median.first) {
                    parent_node->keys[i] = median.first;
                    parent_node->values[i] = median.second;
                    parent_node->children_ids[i+1] = right_node_id;
                    inserted = true;
                    break;
                }

                parent_node->keys[i] = parent_node->keys[i-1];
                parent_node->values[i] = parent_node->values[i-1];
                parent_node->children_ids[i+1] = parent_node->children_ids[i];
            }
            if(inserted == false) {
                parent_node->keys[0] = median.first;
                parent_node->values[0] = median.second;
                parent_node->children_ids[1] = right_node_id;
            }
            parent_node->size += 1;
            tree->save_node(parent_node);
            
            // Update right node
            right_node->size = M/2;
            // right_node->is_leaf_node = children_ids[0] == INVALID_NODE_ID;

            // Copy left nodes to child
            for (int i = 0; i < M/2; i++)
            {
                right_node->keys[i] = keys[i+(M/2)+1];
                right_node->values[i] = values[i+(M/2)+1];
                right_node->children_ids[i] = children_ids[i+(M/2)+1];
            }
            right_node->children_ids[M/2] = children_ids[M];
            tree->save_node(right_node);

            // Update left node (this node)
            size = M/2;
            for (int i = (M/2)+1; i < M+1; i++)
            {
                children_ids[i] = INVALID_NODE_ID;
            }
            tree->save_node(this);

            // Update children references
            for (int i = 0; i < right_node->size+1; i++)
            {
                if(right_node->is_leaf_node() == false) {
                    assert(right_node->children_ids[i] != 0);
                    BTreeNode<M>* child_node = tree->get_node(right_node->children_ids[i]);
                    child_node->parent_id = right_node->node_id;
                    tree->save_node(child_node);
                }
            }

            tree->flush_write();
        }
    }

    bool get(key_type key, value_type* value) {
        if (!is_leaf_node()) {
            // Find lower nodes to insert element into
            bool found_subnode = false;
            // Try nodes
            for (int i = 0; i < size; i++) {
                if (key == keys[i]) {
                    *value = values[i];
                    return true;
                }
                if (key < keys[i]) {
                    return tree->get_node(children_ids[i])->get(key, value);
                }
            } 
            // If no subnode is valid then the value of the key > last key.
            //  so add it to the last b_tree child.
            if(!found_subnode) {
                return tree->get_node(children_ids[size])->get(key, value);
            }
            return false;
        } else {
            for (size_t i = 0; i < size; i++)
            {
                if (key == keys[i]) {
                    *value = values[i];
                    return true;
                }
            }
        }
        return false;
    }

    void put(key_type key, value_type value) {
        assert(parent_id != node_id);
        // On the way down, check for splits and split as needed
        if(size == M) {
            if(VERBOSE_SPLITTING) {
                std::cout << NODE_ID << ": Node is full. Splitting on way down" << std::endl;
            }

            split();
            // Because of split, this might no loger be the best node so insert into parent.
            if(parent_id == INVALID_NODE_ID) {
                this->put(key, value);
                return;
            } else {
                BTreeNode<M>* parent_node = tree->get_node(parent_id);
                parent_node->put(key, value);
                return;
            }
        }


        if (!is_leaf_node()) {
            // Find lower nodes to insert element into
            bool found_subnode = false;
            // Try nodes
            for (int i = 0; i < size; i++) {
                if(key == keys[i]) {
                    values[i] = value;
                    return;
                }
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

        insert_into_leaf(key, value);

        return;
    }

    void set_size(int size) {
        this->size = size;
    }

    void set_parent_id(node_id_t id) {
        this->parent_id = id;
    }

    void set_node_id(node_id_t id) {
        this->node_id = id;
    }

    node_id_t get_parent_id() {
        return this->parent_id;
    }

    node_id_t get_node_id() {
        return this->node_id;
    }

    void set_key_at(int index, key_type key) {
        this->keys[index] = key;
    }

    void set_value_at(int index, value_type value) {
        this->values[index] = value;
    }

    void set_children_id_at(int index, node_id_t node_id) {
        this->children_ids[index] = node_id;
    }

    key_type get_key_at(int index) {
        assert(index < size);
        return this->keys[index];
    }

    value_type get_value_at(int index) {
        assert(index < size);
        return this->values[index];
    }


    node_id_t get_child_id_at(int index) {
        assert(index <= size);
        return this->children_ids[index];
    }

    int get_size() {
        return this->size;
    }

    void print() {
        std::cout << " Parent is " << parent_id << std::endl;
        std::cout << node_id << (is_leaf_node() ? "L" : " ") <<  " : (" << size << ")" << "| ";
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

        if(!is_leaf_node()) {
            for (int i = 0; i < size+1; i ++) {
                if (children_ids[i] != INVALID_NODE_ID) {
                    tree->get_node(children_ids[i])->print();
                }
            }
        }
    }
};


#endif