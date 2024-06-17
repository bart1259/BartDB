
#include "database.h"

Database::Database(const char* database_name)
{
    this->database_name = database_name;

    std::stringstream ss;
    ss << database_name << ".bartdb";
    const char* database_filename_temp = ss.str().c_str();
    database_filename = (char*)malloc(strlen(database_filename_temp) + 1);
    strcpy(database_filename, database_filename_temp);
}

Database::~Database()
{
}

void Database::open() {
    // See if database_name file exists
    if(file_exists(database_filename)) {
        std::cout << "File " << database_filename << " exists" << std::endl;
    } else {
        std::cout << "File " << database_filename << " does not exist" << std::endl;
    }
}