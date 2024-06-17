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

        std::stringstream ss;
        ss << database_name << ".bartdb";
        const char* database_filename_temp = ss.str().c_str();
        database_filename = (char*)malloc(strlen(database_filename_temp) + 1);
        strcpy(database_filename, database_filename_temp);
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
        std::cout << "Root loaded. Has " << root->get_size() << " elements" << std::endl;
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

    void update_header() {
        // Write header file
        char* header = (char*)malloc(PAGE_SIZE);
        std::copy(
            reinterpret_cast<const char*>(&node_count),
            reinterpret_cast<const char*>(&node_count) + sizeof(int),
            header
        );

        fwrite(header, PAGE_SIZE, 1, file);
        delete header;
    }

    node_id_t create_node() override {
        node_count += 1;
        return node_count-1;
    }

    BTreeNode<M>* get_node(node_id_t node_id) override {
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

    BTreeNode<M>* get_root_node() override {
        return root;
    }
};



#endif