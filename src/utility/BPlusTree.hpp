#ifndef TICKETSYSTEM_BPT_HPP
#define TICKETSYSTEM_BPT_HPP

#include <fstream>
#include <iostream>
#include <string>
#include "vector.hpp"

template<class Key, class Value>
class BPlusTree {
private:
    static constexpr int node_size = 200;
    static constexpr int block_size = 1024;

    //son: (,key]
    struct KeyGroup {
        Key key;
        long address = 0;

        friend bool operator<(const KeyGroup &a, const KeyGroup &b) {
            return a.key < b.key;
        }

        friend bool operator==(const KeyGroup &a, const KeyGroup &b) {
            return a.key == b.key;
        }

    public:
        KeyGroup() = default;

        KeyGroup(const Key &key1, const long &address = 0) : key(key1), address(address) {}

        Key GetKey() const {
            return key;
        }

    };

    struct ValueType {
        Key key;
        Value value;

        friend bool operator<(const ValueType &a, const ValueType &b) {
            return a.key < b.key;
        }

        friend bool operator==(const ValueType &a, const ValueType &b) {
            return a.key == b.key;
        }

    public:
        ValueType() = default;

        ValueType(const Key &key1, const Value &value1 = 0) : key(key1), value(value1) {}

        Key GetKey() const {
            return key;
        }

        Value GetValue() const {
            return value;
        }
    };

    struct Node {
        int size = 0;
        bool son_is_block = true;//type of son
        /*
         * -1:normal
         * 0:is_root
         * 1:son_of_root
         */
        int node_type = -1;
        KeyGroup key[node_size];

        Node() = default;

        Node(const KeyGroup &keyGroup1, const KeyGroup &keyGroup2) : size(2), son_is_block(false), node_type(0) {
            key[0] = keyGroup1;
            key[1] = keyGroup2;
        }
    };

    //all the blocks are linked like a linkList
    struct Block {
        int size = 0;
        ValueType storage[block_size];
        long next_block_address = -1;

        Block() = default;

        Block(const Key &key, const Value &value) {
            size = 1;
            storage[0].key = key;
            storage[0].value = value;
        }

    };

    /*
     * address of root_node
     * read into memory when open the file
     * write back when destruct
     */
    long root = 0;
    /*
     * read root_node and its son nodes into memory when construct
     * write back when breakRoot(root changed), breakNode(add new son to root) and destruct
     */
    Node root_node;//root of the tree
    Node son_of_root[node_size];//son of root_node

    Node current_node;
    Block current_block;

    //associated with file when construct the tree
    std::fstream r_w_tree;
    std::fstream r_w_list;

public:
    //associate the tree with file
    BPlusTree(const std::string &file_name, const std::string &list_name) {
        r_w_tree.open(file_name);

        if (!r_w_tree.good()) {//doesn't exist
            r_w_tree.open(file_name, std::ios::out);
            r_w_tree.close();
            r_w_tree.open(file_name);

            r_w_tree.seekp(0);//将指针定位到文件开头
            r_w_tree.write(reinterpret_cast<char *> (&root), sizeof(root));
            root_node.node_type = 0;
            r_w_tree.seekp(0, std::ios::end);
            root = r_w_tree.tellp();
            r_w_tree.write(reinterpret_cast<char *> (&root_node), sizeof(root_node));//root_node may be empty

            r_w_list.open(list_name, std::ios::out);
            r_w_list.close();
            r_w_list.open(list_name);
        } else {

            r_w_list.open(list_name);

            //read root
            r_w_tree.seekg(0);//将指针定位到文件开头
            r_w_tree.read(reinterpret_cast<char *> (&root), sizeof(root));
            //read root node into memory
            ReadNode(root_node, root);
            //if root have son node, read into memory
            if (!root_node.son_is_block) {
                int num = root_node.size;
                for (int i = 0; i < num; ++i) {
                    ReadNode(son_of_root[i], root_node.key[i].address);
                }
            }
        }
    }

    //if root_node changed,changed it in memory
    //write back when destruct
    ~BPlusTree() {
        //write root
        r_w_tree.seekp(0);//将指针定位到文件开头
        r_w_tree.write(reinterpret_cast<char *>(&root), sizeof(root));
        //write root_node
        WriteNode(root_node, root);
        if (!root_node.son_is_block) {
            int num = root_node.size;
            for (int i = 0; i < num; ++i) {
                WriteNode(son_of_root[i], root_node.key[i].address);
            }
        }
    }

    //insert downwards
    //change key when getting down
    //break upwards
    void Insert(const Key &key, const Value &value) {
        if (!root_node.size) {//empty
            Block new_block(key, value);
            ++root_node.size;
            r_w_list.seekp(0, std::ios::end);//the end of the file
            root_node.key[0].key = key;
            root_node.key[0].address = r_w_list.tellp();
            WriteBlock(new_block, root_node.key[0].address);
            return;
        }
        KeyGroup target(key);
        InsertInNode(key, target, value, root_node);
        if (root_node.size == node_size) {//root need to break
            //write son_of_root
            if (!root_node.son_is_block) {
                for (int i = 0; i < node_size; ++i) {
                    son_of_root[i].node_type = -1;
                    WriteNode(son_of_root[i], root_node.key[i].address);
                }
            }
            Node new_node;
            root_node.node_type = new_node.node_type = 1;//son_of_root
            root_node.size = new_node.size = node_size / 2;
            for (int i = 0; i < new_node.size; ++i) {
                new_node.key[i] = root_node.key[new_node.size + i];
            }
            new_node.son_is_block = root_node.son_is_block;
            r_w_tree.seekp(0, std::ios::end);//the end of the file
            Node new_root(KeyGroup(root_node.key[root_node.size - 1].key, root),
                          KeyGroup(new_node.key[new_node.size - 1].key, r_w_tree.tellp()));
            WriteNode(root_node, new_root.key[0].address);
            WriteNode(new_node, new_root.key[1].address);
            //update son_of_root
            son_of_root[0] = root_node;
            son_of_root[1] = new_node;
            r_w_tree.seekp(0, std::ios::end);//the end of the file
            root = r_w_tree.tellp();
            root_node = new_root;
            WriteNode(root_node, root);
        }
    }

    //delete and adjust upwards
    bool Delete(const Key &key) {
        long iter = 0;
        bool adjust_flag = true;
        KeyGroup target(key);
        bool flag = RemoveInNode(key, target, iter, root_node, adjust_flag);
        if (root_node.size == 1) {//root need to adjust
            if (!root_node.son_is_block) {
                //change root
                root = root_node.key[0].address;
                root_node = son_of_root[0];
                root_node.node_type = 0;
                //update son_of_root
                if (!root_node.son_is_block)
                    for (int i = 0; i < root_node.size; ++i) {
                        ReadNode(son_of_root[i], root_node.key[i].address);
                        son_of_root[i].node_type = 1;
                    }
            }
        }
        return flag;
    }


    template<class Compare>
    void Find(const Key &key, const Compare &cmp, sjtu::vector<Value> &vec) {
        current_node = root_node;//start from root
        long iter;
        KeyGroup target(key);
        FindNode(target, iter, cmp, vec);
    }

    void Find(const Key &key) {
        current_node = root_node;//start from root
        long iter;
        KeyGroup target(key);
        FindNode(target, iter);
    }


private:

    template<class Array>
    int BinarySearch(Array array[], int l, int r, const Array &target) {
        int mid, ans = -1;
        while (l <= r) {
            mid = (l + r) >> 1;
            if (array[mid] < target) {
                l = mid + 1;
            } else {
                r = mid - 1;
                ans = mid;
            }
        }
        return ans;
    }

    template<class Compare>
    int BinarySearch(KeyGroup array[], int l, int r, const KeyGroup &target, const Compare &cmp) {
        int mid, ans = -1;
        while (l <= r) {
            mid = (l + r) >> 1;
            if (cmp(array[mid].key, target.key)) {
                l = mid + 1;
            } else {
                r = mid - 1;
                ans = mid;
            }
        }
        return ans;
    }

    template<class Compare>
    int BinarySearch(ValueType array[], int l, int r, const ValueType &target, const Compare &cmp) {
        int mid, ans = -1;
        while (l <= r) {
            mid = (l + r) >> 1;
            if (cmp(array[mid].key, target.key)) {
                l = mid + 1;
            } else {
                r = mid - 1;
                ans = mid;
            }
        }
        return ans;
    }

    template<class T>
    T Max(const T &a, const T &b) {
        return b < a ? a : b;
    }

    inline void ReadNode(Node &current, const long &iter) {
        r_w_tree.seekg(iter);
        r_w_tree.read(reinterpret_cast<char *> (&current), sizeof(Node));
    }

    inline void WriteNode(Node current, const long &iter) {
        r_w_tree.seekp(iter);
        r_w_tree.write(reinterpret_cast<char *> (&current), sizeof(Node));
    }

    inline void ReadBlock(Block &current, const long &iter) {
        r_w_list.seekg(iter);
        r_w_list.read(reinterpret_cast<char *> (&current), sizeof(Block));
    }

    inline void WriteBlock(Block current, const long &iter) {
        r_w_list.seekp(iter);
        r_w_list.write(reinterpret_cast<char *> (&current), sizeof(Block));
    }

    template<class Compare>
    void GetEle(const ValueType &target, int index_in_block, const Compare &cmp, sjtu::vector<Value> &vec) {
        while (index_in_block < current_block.size &&
               !(cmp(current_block.storage[index_in_block].key, target.key) ||
                 cmp(target.key, current_block.storage[index_in_block].key))) {
            vec.push_back(current_block.storage[index_in_block].value);
            ++index_in_block;
        }
        if (index_in_block == current_block.size && current_block.next_block_address > 0) {
            long iter = current_block.next_block_address;
            ReadBlock(current_block, iter);
            GetEle(target, 0, cmp, vec);
        }
    }

    template<class Compare>
    void FindFirstEle(const Key &key, long &iter, const Compare &cmp, sjtu::vector<Value> &vec) {
        ReadBlock(current_block, iter);
        ValueType target(key);
        int index_in_block = BinarySearch(current_block.storage, 0, current_block.size - 1, target, cmp);
        while (index_in_block == -1 && current_block.next_block_address != -1) {
            ReadBlock(current_block, current_block.next_block_address);
            index_in_block = BinarySearch(current_block.storage, 0, current_block.size - 1, target, cmp);
        }
        if (!(cmp(current_block.storage[index_in_block].key, target.key) ||
              cmp(target.key, current_block.storage[index_in_block].key))) {
            GetEle(target, index_in_block, cmp, vec);
        } else return;
    }

    /*
     * based on index
      * recursive find the node
      * not exist return false
      * exist return true
      */
    template<class Compare>
    void FindNode(const KeyGroup &target, long &iter, const Compare &cmp, sjtu::vector<Value> &vec) {
        int index = BinarySearch(current_node.key, 0, current_node.size - 1, target, cmp);
        if (index == -1) index = current_node.size - 1;
        iter = current_node.key[index].address;
        //end of recursion
        if (current_node.son_is_block) {
            FindFirstEle(target.key, iter, cmp, vec);
            return;
        }
        if (!current_node.node_type) {//is_root
            current_node = son_of_root[index];
        } else ReadNode(current_node, iter);
        FindNode(target, iter, cmp, vec);
    }

    void BreakNode(Node &current, Node &father, int index) {
        Node new_node;
        new_node.node_type = current.node_type;
        current.size = new_node.size = node_size / 2;
        for (int i = 0; i < new_node.size; ++i) {
            new_node.key[i] = current.key[new_node.size + i];
        }
        new_node.son_is_block = current.son_is_block;
        r_w_tree.seekp(0, std::ios::end);//the end of the file
        for (int i = father.size; i > index + 1; --i) {
            father.key[i] = father.key[i - 1];
        }
        father.key[index].key = current.key[current.size - 1].key;
        father.key[index + 1].key = new_node.key[new_node.size - 1].key;
        father.key[index + 1].address = r_w_tree.tellp();
        if (current.node_type < 0) WriteNode(current, father.key[index].address);
        WriteNode(new_node, father.key[index + 1].address);
        if (new_node.node_type > 0) {
            for (int i = father.size; i > index + 1; --i) {
                son_of_root[i] = son_of_root[i - 1];
            }
            son_of_root[index + 1] = new_node;
        }
        ++father.size;
    }

    void BreakBlock(Node &father, int index) {
        Block new_block;
        new_block.size = block_size / 2;
        current_block.size = block_size / 2;
        for (int i = 0; i < new_block.size; ++i) {
            new_block.storage[i] = current_block.storage[new_block.size + i];
        }
        new_block.next_block_address = current_block.next_block_address;
        r_w_list.seekp(0, std::ios::end);//the end of the file
        current_block.next_block_address = r_w_list.tellp();
        WriteBlock(new_block, current_block.next_block_address);
        WriteBlock(current_block, father.key[index].address);
        for (int i = father.size; i > index + 1; --i) {
            father.key[i] = father.key[i - 1];
        }
        father.key[index].key = current_block.storage[current_block.size - 1].key;
        father.key[index + 1].key = new_block.storage[new_block.size - 1].key;
        father.key[index + 1].address = current_block.next_block_address;
        ++father.size;
    }


    void InsertInNode(const Key &key, const KeyGroup &target, const Value &value, Node &current, long iter = -1) {
        bool write_current_flag = false;//if current is changed and is not root or son_of_root
        int index = BinarySearch(current.key, 0, current.size - 1, target);
        if (index == -1) {
            current.key[current.size - 1].key = key;
            write_current_flag = true;
            index = current.size - 1;
        }
        if (current.son_is_block) {
            ReadBlock(current_block, current.key[index].address);
            InsertInBlock(key, value);
            if (current_block.size == block_size) {
                BreakBlock(current, index);
                if (current.node_type < 0) write_current_flag = true;
            } else WriteBlock(current_block, current.key[index].address);
        } else {
            if (!current.node_type) {//is root
                InsertInNode(key, target, value, son_of_root[index]);
                if (son_of_root[index].size == node_size) {
                    BreakNode(son_of_root[index], current, index);
                }
            } else {
                Node next_node;
                ReadNode(next_node, current.key[index].address);
                InsertInNode(key, target, value, next_node, current.key[index].address);
                if (next_node.size == node_size) {
                    BreakNode(next_node, current, index);
                    if (current.node_type < 0) write_current_flag = true;
                }
            }
        }
        if (write_current_flag && current.node_type < 0)WriteNode(current, iter);
    }

    void InsertInBlock(const Key &key, const Value &value) {
        ValueType target(key, value);
        int index_in_block = BinarySearch(current_block.storage, 0, current_block.size - 1, target);
        if (index_in_block == -1)index_in_block = current_block.size;
        else if (current_block.storage[index_in_block].key == key) return;
        for (int i = current_block.size; i > index_in_block; --i) {
            current_block.storage[i] = current_block.storage[i - 1];
        }
        current_block.storage[index_in_block] = target;
        ++current_block.size;
    }

    void AdjustRemoveInNode(Node &current, Node &father, int index, bool &adjust_flag) {
        Node pre_node, next_node;
        if (index) {
            if (!father.node_type) {
                pre_node = son_of_root[index - 1];
            } else
                ReadNode(pre_node, father.key[index - 1].address);
        }
        if (index < father.size - 1) {
            if (!father.node_type) {
                next_node = son_of_root[index + 1];
            } else
                ReadNode(next_node, father.key[index + 1].address);
        }
        if (pre_node.size > node_size / 2) {//borrow from the pre
            //update array
            int num = (current.size + pre_node.size) >> 1;
            int move = pre_node.size - num;
            for (int i = current.size - 1; i >= 0; --i) {
                current.key[i + move] = current.key[i];
            }
            for (int i = 0; i < move; ++i) {
                current.key[i] = pre_node.key[num + i];
            }
            pre_node.size = num;
            current.size += move;
            //update key
            father.key[index - 1].key = pre_node.key[num - 1].key;
            if (pre_node.node_type == 1) {
                son_of_root[index - 1] = pre_node;
                son_of_root[index] = current;
            }
            if (current.node_type < 0) {
                WriteNode(current, father.key[index].address);
                WriteNode(pre_node, father.key[index - 1].address);
            }
            adjust_flag = false;
            return;
        }
        if (next_node.size > node_size / 2) {//borrow from next
            //update array
            int num = (current.size + next_node.size) >> 1;
            int move = next_node.size - num;
            for (int i = 0; i < move; ++i) {
                current.key[current.size + i] = next_node.key[i];
            }
            current.size += move;
            next_node.size = num;
            for (int i = 0; i < num; ++i) {
                next_node.key[i] = next_node.key[i + move];
            }
            father.key[index].key = current.key[current.size - 1].key;
            if (next_node.node_type == 1) {
                son_of_root[index + 1] = next_node;
                son_of_root[index] = current;
            }
            if (current.node_type < 0) {
                WriteNode(current, father.key[index].address);
                WriteNode(next_node, father.key[index + 1].address);
            }
            adjust_flag = false;
            return;
        }
        //merge
        //try the next one
        if (next_node.size) {//exist
            int prime_size = current.size;
            for (int i = 0; i < next_node.size; ++i) {
                current.key[prime_size + i] = next_node.key[i];
            }
            current.size += next_node.size;
            --father.size;
            father.key[index].key = current.key[current.size - 1].key;
            for (int i = index + 1; i < father.size; ++i) {
                father.key[i] = father.key[i + 1];
            }
            if (!father.node_type) {//father is root
                for (int i = index + 1; i < father.size; ++i) {
                    son_of_root[i] = son_of_root[i + 1];
                }
            }
            if (current.node_type < 0) WriteNode(current, father.key[index].address);
            if (father.size * 2 >= node_size) adjust_flag = false;
            return;
        }
        if (pre_node.size) {//merge with pre
            int prime_size = pre_node.size;
            for (int i = 0; i < current.size; ++i) {
                pre_node.key[prime_size + i] = current.key[i];
            }
            pre_node.size += current.size;
            --father.size;
            father.key[index - 1].key = pre_node.key[pre_node.size - 1].key;
            for (int i = index; i < father.size; ++i) {
                father.key[i] = father.key[i + 1];
            }
            if (!father.node_type) {//father is root
                son_of_root[index - 1] = pre_node;
                for (int i = index; i < father.size; ++i) {
                    son_of_root[i] = son_of_root[i + 1];
                }
            }
            if (pre_node.node_type < 0) WriteNode(pre_node, father.key[index - 1].address);
            if (father.size * 2 >= node_size) adjust_flag = false;
            return;
        }
    }

    /*
     * block has ele <= block_size/2
     * borrow?
     * merge? always try to merge with the one after it
     */
    void AdjustRemoveInBlock(Node &father, int index, bool &adjust_flag) {
        Block pre_block, next_block;
        if (index) {
            ReadBlock(pre_block, father.key[index - 1].address);
        }
        if (index < father.size - 1) {
            ReadBlock(next_block, father.key[index + 1].address);
        }
        if (pre_block.size > block_size / 2) {//borrow from the pre
            //update array
            int num = (current_block.size + pre_block.size) >> 1;
            int move = pre_block.size - num;
            for (int i = current_block.size - 1; i >= 0; --i) {
                current_block.storage[i + move] = current_block.storage[i];
            }
            for (int i = 0; i < move; ++i) {
                current_block.storage[i] = pre_block.storage[num + i];
            }
            pre_block.size = num;
            current_block.size += move;
            //update key
            father.key[index - 1].key = pre_block.storage[num - 1].key;
            WriteBlock(current_block, father.key[index].address);
            WriteBlock(pre_block, father.key[index - 1].address);
            adjust_flag = false;
            return;
        }
        if (next_block.size > block_size / 2) {//borrow from next
            //update array
            int num = (current_block.size + next_block.size) >> 1;
            int move = next_block.size - num;
            for (int i = 0; i < move; ++i) {
                current_block.storage[current_block.size + i] = next_block.storage[i];
            }
            current_block.size += move;
            next_block.size = num;
            for (int i = 0; i < num; ++i) {
                next_block.storage[i] = next_block.storage[i + move];
            }
            father.key[index].key = current_block.storage[current_block.size - 1].key;
            WriteBlock(current_block, father.key[index].address);
            WriteBlock(next_block, father.key[index + 1].address);
            adjust_flag = false;
            return;
        }
        //merge
        //try the next one
        if (next_block.size) {//exist
            int prime_size = current_block.size;
            for (int i = 0; i < next_block.size; ++i) {
                current_block.storage[prime_size + i] = next_block.storage[i];
            }
            current_block.size += next_block.size;
            current_block.next_block_address = next_block.next_block_address;
            --father.size;
            father.key[index].key = current_block.storage[current_block.size - 1].key;
            for (int i = index + 1; i < father.size; ++i) {
                father.key[i] = father.key[i + 1];
            }
            WriteBlock(current_block, father.key[index].address);
            if (father.size * 2 >= node_size) adjust_flag = false;
            return;
        }
        if (pre_block.size) {//merge with pre
            int prime_size = pre_block.size;
            for (int i = 0; i < current_block.size; ++i) {
                pre_block.storage[prime_size + i] = current_block.storage[i];
            }
            pre_block.size += current_block.size;
            pre_block.next_block_address = current_block.next_block_address;
            --father.size;
            father.key[index - 1].key = pre_block.storage[pre_block.size - 1].key;
            for (int i = index; i < father.size; ++i) {
                father.key[i] = father.key[i + 1];
            }
            WriteBlock(pre_block, father.key[index - 1].address);
            if (father.size * 2 >= node_size) adjust_flag = false;
            return;
        }
        WriteBlock(current_block, father.key[index].address);
    }

    /*
     * recursive remove
     * return false if ele with given key doesn't exist
     * downwards
     * MergeBlock or MergeNode if necessary(use adjust function)
     *
     * adjust_flag==true:adjust upwards
     *              false:"stop"(node remain unchanged)
     */
    bool RemoveInNode(const Key &key, const KeyGroup &target, long &iter, Node &current, bool &adjust_flag) {
        if (current.key[current.size - 1].GetKey() < key) {//exceed
            return false;
        }
        //the index of the section
        int index = BinarySearch(current.key, 0, current.size - 1, target);
        iter = current.key[index].address;
        //end of recursion
        if (current.son_is_block) {
            if (!RemoveInBlock(key, iter, adjust_flag)) return false;//doesn't exist
            if (adjust_flag) AdjustRemoveInBlock(current, index, adjust_flag);
        } else {
            //next layer
            if (!current.node_type) {
                if (!RemoveInNode(key, target, iter, son_of_root[index], adjust_flag))return false;
                if (adjust_flag) AdjustRemoveInNode(son_of_root[index], current, index, adjust_flag);
                if (!adjust_flag) current.key[index].key = son_of_root[index].key[son_of_root[index].size - 1].key;
            } else {
                Node next;
                ReadNode(next, iter);
                if (!RemoveInNode(key, target, iter, next, adjust_flag))return false;
                if (adjust_flag) AdjustRemoveInNode(next, current, index, adjust_flag);
                else {
                    if (next.node_type < 0) WriteNode(next, current.key[index].address);//write back
                }
                if (!adjust_flag) current.key[index].key = next.key[next.size - 1].key;
            }
        }
        return true;
    }

    bool RemoveInBlock(const Key &key, long &iter, bool &adjust_flag) {
        ReadBlock(current_block, iter);
        ValueType target(key);
        int index_in_block = BinarySearch(current_block.storage, 0, current_block.size - 1, target);
        if (current_block.storage[index_in_block].key == key) {//the ele to be removed
            --current_block.size;
            for (int i = index_in_block; i < current_block.size; ++i) {
                current_block.storage[i] = current_block.storage[i + 1];
            }
            if (current_block.size * 2 >= block_size) {
                adjust_flag = false;
                WriteBlock(current_block, iter);//if block need to adjust don't write
            }
        } else {
            adjust_flag = false;
            return false;
        }
        return true;
    }
};

#endif //TICKETSYSTEM_BPT_HPP