

#ifndef BPLUSTREE_KEY_HPP
#define BPLUSTREE_KEY_HPP

#include <cstring>
#include <iostream>

struct Key {
    char index[64];
    int value = 0;

    Key() {
        memset(index, 0, sizeof(index));
    }

    Key(char *id, const int &value = 0) : value(value) {
        strcpy(index, id);
    }

    Key(const Key &other) {
        strcpy(index, other.index);
        value = other.value;
    }

    //GetVal
    int GetVal() const {
        return value;
    }

    void Print() const {
        std::cout << value << " ";
    }

    bool operator<(const Key &other) const {
        if (strcmp(index, other.index) != 0)return strcmp(index, other.index) < 0;
        return value < other.value;
    }

    bool operator==(const Key &key) const {
        if (strcmp(index, key.index) != 0)return false;
        if (key.value != value)return false;
        return true;
    }

    bool operator<=(const Key &Key) const {
        return !(Key < *this);
    }

    Key &operator=(const std::pair<Key, bool> &key) {
        *this = key.first;
        return *this;
    }
};

//关于key的compare类
struct cmp {
    bool operator()(const Key &a, const Key &b) const {
        return strcmp(a.index, b.index) < 0;
    }
};

#endif //BPLUSTREE_KEY_HPP
