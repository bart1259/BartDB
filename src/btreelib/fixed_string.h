#ifndef FIXED_STRING_H
#define FIXED_STRING_H

#include <array>
#include <ostream>
#include <string>
#include <cstring>

/**
 * A cusotm class for a string of fixed size (std::string does not have a defined size at compile time)
 */
template <int L>
class FixedString {
protected:
    std::array<char, L> data;

public:
    FixedString() {
        data.fill(0);
    }
    ~FixedString() {

    }
    FixedString(char* const & str) {
        data.fill(0);
        std::memcpy(&data, str, (L < strlen(str) ? L : strlen(str)));
    }

    FixedString(const char* const & str) {
        data.fill(0);
        std::memcpy(&data, str, (L < strlen(str) ? L : strlen(str)));
    }

    auto begin() const {
        return data.begin();
    }

    auto end() const {
        return data.end();
    }


    char at(int index) const {
        return data[index];
    }

    char* c_str() const {
        char* str = (char*)malloc(L+1);
        std::fill(str, str+L+1, '\0');
        std::copy(&(data[0]), &(data[L-1]), str);
        return str;        
    }

    // Override importantn operators to make our lives easier

    // Friend std::cout operator
    friend std::ostream& operator<<(std::ostream& os, FixedString const & fs) {
        for (int i = 0; i < L; i++)
        {
            if (fs.at(i) == 0)
                break;
            os << fs.at(i);
        }
        return os;
    }

    template<int L2>
    bool operator<(const FixedString<L2> other) const
    { 
        int maxLength = (L < L2 ? L : L2);
        for (int i = 0; i < maxLength; i++) {
            if (this->data[i] < other.at(i))
                return true;
            if (this->data[i] > other.at(i))
                return false;
        }
        return L < L2;
    } 

    template<int L2>
    bool operator>(const FixedString<L2> other) const
    { 
        int maxLength = (L < L2 ? L : L2);
        for (int i = 0; i < maxLength; i++) {
            if (this->data[i] > other.at(i))
                return true;
            if (this->data[i] < other.at(i))
                return false;
        }
        return L > L2;
    } 

    template<int L2>
    bool operator==(const FixedString<L2> other) const
    { 
        if (L != L2) {
            return false;
        }
        int maxLength = (L < L2 ? L : L2);
        for (int i = 0; i < maxLength; i++) {
            if (this->data[i] != other.at(i))
                return false;
        }
        return true;
    } 
};


#endif