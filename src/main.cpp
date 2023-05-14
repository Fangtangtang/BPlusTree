#include <iostream>
#include "head-file/key.hpp"
#include "utility/bpt.hpp"

using namespace std;

bool CMP(const int &a, const int &b) {
    return a > b;
}

int main() {
//    freopen("my.out", "w", stdout);
    BPlusTree<Key, int, cmp, cmp, cmp> tree("my_file", "list_file");
    int n;
    int cnt = 0;
    cin >> n;
    string cmd;
    char index[65];
    int value;
    while (n--) {
        ++cnt;
        cin >> cmd;
        cin >> index;
//        if (cnt == 635) {
//            cout << "!!!\n";
//            cout<<cnt<<"-----------------------\n";
//            tree.Print();
//            cout << "------------------------------\n";
//        }
//        Index index1(index);
        if (cmd == "insert") {
            cin >> value;
            Key key(index, value);
            tree.Insert(key, value);
        }
        if (cmd == "delete") {
            cin >> value;
            Key key(index, value);
            tree.Delete(key);
        }
        //index-based find
        if (cmd == "find") {
            Key key(index);
//            PrintVector(tree.StrictFind(key), CMP, true);
            bool flag = tree.StrictFind(key).traverse();
            if (!flag) cout << "null";
            std::cout << "\n";
        }
//        tree.Print();
    }
//    tree.Print();
    return 0;
}
