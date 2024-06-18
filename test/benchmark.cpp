#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "b_tree.h"
#include "b_tree_node.h"
#include "fileio.h"
#include <iomanip>
#include "database.h"
#include "memory_storage_engine.h"
#include <chrono>


char* generate_random_string() {
    const int SIZE = 16;
    char* strg = (char*)malloc((SIZE + 1) * sizeof(char));
    for (int i = 0; i < SIZE; i++)
    {
        strg[i] = (char)(65 + (std::rand() % 26));
    }
    strg[SIZE] = '\0';
    return strg;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        std::cout << 
            "Usage: " << std::endl <<
            "  ./benchmark create large_db 100000" << std::endl <<
            "  ./benchmark read large_db 10" << std::endl << 
            "  ./benchmark put large_db 10" << std::endl;
        return 0;
    }

    if(strcmp(argv[1], "create") == 0) {
        assert(argc == 4);
        // Create a new database
        char* database_name = argv[2];
        int entry_count = atoi(argv[3]);

        std::cout << "Creating database " << database_name << " with " << entry_count << " entries." << std::endl;

        Database<31>* db = new Database<31>(database_name);
        BTree<31>* n = new BTree<31>(db);
        db->open();

        std::srand(0);
        std::vector<char*> random_strings; 
        for (int i = 0; i < entry_count; i++)
        {
            char* rand_string = generate_random_string();
            random_strings.push_back(rand_string);
        }

        // Insert in random order
        for (int i = 0; i < random_strings.size(); i++)
        {
            if(i%1000 == 0){
                std::cout << 100 * i / (float)entry_count << "%" << std::endl;
            }
            n->put(key_type(random_strings[i]), key_type(random_strings[i]));
        }
        std::cout << "Database creation complete!" << std::endl;
        db->close();
        return 0;
    }

    if(strcmp(argv[1], "read") == 0 || strcmp(argv[1], "put") == 0) {
        assert(argc == 4);

        bool is_reading_rask = strcmp(argv[1], "read") == 0 ;

        // Create a new database
        char* database_name = argv[2];
        int read_count = atoi(argv[3]);

        std::cout << "Reading database " << database_name << " " << read_count << " times." << std::endl;

        Database<31>* db = new Database<31>(database_name);
        BTree<31>* n = new BTree<31>(db);
        db->open();

        std::srand(time(NULL));
        std::vector<char*> random_strings; 
        for (int i = 0; i < read_count; i++)
        {
            char* rand_string = generate_random_string();
            random_strings.push_back(rand_string);
        }

        // Perform random reads
        for (int i = 0; i < random_strings.size(); i++)
        {
            value_type v;

            // Timing code brrowed from https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
            using std::chrono::high_resolution_clock;
            using std::chrono::duration_cast;
            using std::chrono::duration;
            using std::chrono::milliseconds;

            auto t1 = high_resolution_clock::now();
            if(is_reading_rask) { 
                n->get(key_type(random_strings[i]), &v);
            } else {
                n->put(key_type(random_strings[i]), value_type(random_strings[i]));
            }
            auto t2 = high_resolution_clock::now();

            auto ms_int = duration_cast<milliseconds>(t2 - t1);
            duration<double, std::milli> ms_double = t2 - t1;

            std::cout << ms_double.count() << std::endl;
        }
        std::cout << "Database creation complete!" << std::endl;
        db->close();
        return 0;
    }

    return 0;
}