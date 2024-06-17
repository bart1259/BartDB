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

#define BTREE_NODE_SIZE 31

inline bool file_exists (const char* name) {
    return ( access( name, F_OK ) != -1 );
}

class Database
{
private:
    const char* database_name;
    char* database_filename;
public:
    Database(const char* database_name);
    ~Database();

    void open();
};


#endif