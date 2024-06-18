#ifndef DATABASE_H
#define DATABASE_H

#include "b_tree.h"
#include <filesystem>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <string>

#define BTREE_NODE_SIZE 31

inline bool file_exists (const char* name) {
    return ( access( name, F_OK ) != -1 );
}

template <int M>
class Database : public AbstractStorageEngine<M>
{
private:
    const char* database_name;
    char* database_filename;
    FILE* file = nullptr;
    int node_count;
    BTreeNode<M>* root = nullptr;
public:
    Database(const char* database_name) {
        this->database_name = database_name;
        database_filename = (char*)malloc(strlen(database_name) + strlen(".bartdb") + 1);
        strcpy(database_filename, database_name);
        strcat(database_filename, ".bartdb");
    }
    ~Database() {

    }

    void read_head() {
        // Read first 4 bytes and convert to an int which is node count.
        char buffer[4];
        fread(buffer, 4, 1, file);

        std::copy(
            buffer,
            buffer + sizeof(int),
            reinterpret_cast<char*>(&node_count)
        );
        std::cout << "Database has " << node_count << " nodes" << std::endl;

        // Read root TODO: Make this call read_node
        char root_buffer[PAGE_SIZE];
        fseek(file, PAGE_SIZE * 1, SEEK_SET);
        fread(root_buffer, PAGE_SIZE, 1, file);
        root = bytes_to_b_tree_node(this->tree, root_buffer);
    }

    void open() {
        // See if database_name file exists
        if(file_exists(database_filename)) {
            std::cout << "File " << database_filename << " exists" << std::endl;
            file = fopen(database_filename, "rb+");
            read_head();
        } else {
            std::cout << "File " << database_filename << " does not exist" << std::endl;
            FILE* f = fopen(database_filename ,"a");
            fclose(f);

            file = fopen(database_filename, "rb+");
            root = new BTreeNode<M>(this->tree);
            root->set_parent_id(INVALID_NODE_ID);
            root->set_node_id(0);
            node_count += 1;

            save_node(root);
        }
    }

    void close() {
        update_header();
        fclose(file);
    }

    void update_header() {
        // Write header file
        char* header = (char*)malloc(PAGE_SIZE);
        std::copy(
            reinterpret_cast<const char*>(&node_count),
            reinterpret_cast<const char*>(&node_count) + sizeof(int),
            header
        );

        fseek(file, 0, SEEK_SET);
        fwrite(header, PAGE_SIZE, 1, file);
        delete header;
    }

    node_id_t create_node() override {
        // Create empty node
        BTreeNode<M> node = BTreeNode<M>(this->tree);
        node.set_node_id(node_count);
        this->save_node(&node);
        node_count += 1;
        update_header();
        return node_count-1;
    }

    BTreeNode<M>* get_node(node_id_t node_id) override {
        assert(node_id < node_count);
        char node_buffer[PAGE_SIZE];
        fseek(file, PAGE_SIZE * (node_id + 1), SEEK_SET);
        fread(node_buffer, PAGE_SIZE, 1, file);
        
        BTreeNode<M>* node = bytes_to_b_tree_node(this->tree, node_buffer);
        node->set_node_id(node_id);
        return node;
    }

    void save_node(BTreeNode<M>* node) override {
        char* bytes = b_tree_node_to_bytes(node);
        fseek(file, (node->get_node_id()+1)*PAGE_SIZE, SEEK_SET);
        fwrite(bytes, PAGE_SIZE, 1, file);
        delete bytes;
    }

    void flush_write() {
        fsync(fileno(file));
    }

    BTreeNode<M>* get_root_node() override {
        return root;
    }
};



#endif