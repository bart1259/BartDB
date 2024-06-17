#ifndef FILE_IO_H
#define FILE_IO_H

#include "b_tree_node.h"

#define PAGE_SIZE 4096 // in bytes

template <int M>
char* b_tree_node_to_bytes(BTreeNode<M>* node) {
    char* data = (char*)malloc(PAGE_SIZE);

    node_id_t node_id = node->get_node_id();
    node_id_t parent_id = node->get_parent_id();
    int size = node->get_size();

    int offset = 0;

    std::copy(
        reinterpret_cast<const char*>(&node_id),
        reinterpret_cast<const char*>(&node_id) + sizeof(node_id_t),
        &data[offset]
    );
    offset += sizeof(node_id_t);

    std::copy(
        reinterpret_cast<const char*>(&parent_id),
        reinterpret_cast<const char*>(&parent_id) + sizeof(node_id_t),
        &data[offset]
    );
    offset += sizeof(node_id_t);

    std::copy(
        reinterpret_cast<const char*>(&size),
        reinterpret_cast<const char*>(&size) + sizeof(int),
        &data[offset]
    );
    offset += sizeof(int);

    int node_capacity = M;
    std::copy(
        reinterpret_cast<const char*>(&node_capacity),
        reinterpret_cast<const char*>(&node_capacity) + sizeof(int),
        &data[offset]
    );
    offset += sizeof(int);

    // Add all children IDs
    for (size_t i = 0; i < size+1; i++)
    {
        node_id_t child_id = node->get_child_id_at(i);
        std::copy(
            reinterpret_cast<const char*>(&child_id),
            reinterpret_cast<const char*>(&child_id) + sizeof(node_id_t),
            &data[offset]
        );
        offset += sizeof(node_id_t);
    }

    // Add all keys and values
    for (size_t i = 0; i < size; i++)
    {
        key_type key = node->get_key_at(i);
        std::copy(
            key.begin(),
            key.end(),
            &data[offset]
        );
        offset += 64;
    }
    for (size_t i = 0; i < size; i++)
    {
        value_type value = node->get_value_at(i);
        std::copy(
            value.begin(),
            value.end(),
            &data[offset]
        );
        offset += 64;
    }

    return data;
}

template <int M>
BTreeNode<M>* bytes_to_b_tree_node(BTree<M>* tree, char* data) {
    BTreeNode<M>* node = new BTreeNode<M>(tree);

    int offset = 0;

    node_id_t node_id;
    std::copy(
        &data[offset],
        &data[offset] + sizeof(node_id_t),
        reinterpret_cast<char*>(&node_id)
    );
    node->set_node_id(node_id);
    offset += sizeof(node_id_t);

    node_id_t parent_id;
    std::copy(
        &data[offset],
        &data[offset] + sizeof(node_id_t),
        reinterpret_cast<char*>(&parent_id)
    );
    node->set_parent_id(parent_id);
    offset += sizeof(node_id_t);

    int size;
    std::copy(
        &data[offset],
        &data[offset] + sizeof(int),
        reinterpret_cast<char*>(&size)
    );
    node->set_size(size);
    offset += sizeof(int);

    int node_capacity;
    std::copy(
        &data[offset],
        &data[offset] + sizeof(int),
        reinterpret_cast<char*>(&node_capacity)
    );
    offset += sizeof(int);

    // Add all children IDs
    for (size_t i = 0; i < size+1; i++)
    {
        node_id_t child_id;
        std::copy(
            &data[offset],
            &data[offset] + sizeof(node_id_t),
            reinterpret_cast<char*>(&child_id)
        );
        node->set_children_id_at(i, child_id);
        offset += sizeof(node_id_t);
    }

    // Add all keys and values
    for (size_t i = 0; i < size; i++)
    {
        char* key = (char*)malloc(64);
        std::copy(
            &data[offset], 
            &data[offset] + 64,
            reinterpret_cast<char*>(key)
        );
        node->set_key_at(i, key);
        offset += 64;
    }
    for (size_t i = 0; i < size; i++)
    {
        char* value = (char*)malloc(64);
        std::copy(
            &data[offset], 
            &data[offset] + 64,
            reinterpret_cast<char*>(value)
        );
        node->set_value_at(i, value);
        offset += 64;
    }

    return node;
}

#endif