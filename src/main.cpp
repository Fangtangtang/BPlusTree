#include <iostream>
#include "head-file/key.hpp"
#include "utility/bpt.hpp"

using namespace std;

bool CMP(const int &a, const int &b) {
    return a > b;
}

FileManager<int> fileManager("list_file");

bool print(sjtu::vector<long> vec, const string &str) {
    if (vec.empty()) return false;
    auto iter = vec.begin();
    int value;
    while (iter != vec.end()) {
        fileManager.ReadEle(*iter, value);
        cout << value << ' ';
        ++iter;
    }
    return true;
}

int main() {
    freopen("my.out", "w", stdout);
    BPlusTree<Key, int, cmp, cmp, cmp> tree("my_file");
    int n;
    int cnt = 0;
    cin >> n;
    string cmd;
    char index[64];
    int value;
    while (n--) {
        ++cnt;
        cin >> cmd;
        cin >> index;
        if (cmd == "insert") {
            cin >> value;
            Key key(index, value);
            tree.Insert(key, value, fileManager);
        }
        if (cmd == "delete") {
            cin >> value;
            Key key(index, value);
            tree.Delete(key);
        }
        if (cmd == "find") {
            Key key(index);
            bool flag = print(tree.StrictFind(key), "list_file");
            if (!flag) cout << "null";
            std::cout << "\n";
        }
    }
    return 0;
}
