#ifndef FIXED_STRING_H
#define FIXED_STRING_H

#include <array>
#include <ostream>
#include <string>

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