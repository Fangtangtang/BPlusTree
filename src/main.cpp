#include <iostream>
#include "head-file/key.hpp"
//#include "utility/bpt.hpp"
#include "utility/BPlusTree.hpp"

using namespace std;

FileManager<int> fileManager("list_file");

const cmp1 strict;
const cmp2 weak;

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
    BPlusTree<Key, int> tree("my_file", "list_file");
    //将iostream和stdio解绑
    ios_base::sync_with_stdio(false);
    //将输入输出流解绑
    cin.tie(nullptr);
    cout.tie(nullptr);
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
            tree.Insert(key, value);
        }
        if (cmd == "delete") {
            cin >> value;
            Key key(index, value);
            tree.Delete(key);
        }
        if (cmd == "find") {
            Key key(index);
            sjtu::vector<int> vec;
            tree.Find(key, weak, vec);
//            bool flag = print(vec, "list_file");
//            if (!flag) cout << "null";
            if (vec.empty()) cout << "null";
            else {
                int size = vec.size();
                for (int i = 0; i < size; ++i) {
                    cout << vec[i] << ' ';
                }
            }

            std::cout << "\n";
        }
    }
    return 0;
}
