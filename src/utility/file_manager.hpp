/*
 * FILE_MANAGER
 * used to read and write file directly
 * all the ele stored are of same type
 *
 * different managers can be associated with the same file
 */

#ifndef TICKETSYSTEM_FILE_MANAGER_HPP
#define TICKETSYSTEM_FILE_MANAGER_HPP

#include <fstream>
#include <string>

template<class ValueType>
class FileManager {
    static constexpr size_t value_size = sizeof(ValueType);
    std::fstream r_w_file;
    std::string file;
public:
    explicit FileManager(const std::string &file_name) : file(file_name) {
        r_w_file.open(file_name);
        if (!r_w_file.good()) {//doesn't exist
            r_w_file.open(file_name, std::ios::out);
//            r_w_file.close();
//            r_w_file.open(file_name);

            r_w_file.seekp(0);//将指针定位到文件开头
        } else {
            r_w_file.seekg(0);
        }
        r_w_file.close();
    }

    ~FileManager() {
//        r_w_file.close();
    }

    //return move_num-1 th ele behind start_addr
    void ReadEle(const long &start_addr, const int &move_num, ValueType &valueType) {
        r_w_file.seekg(start_addr + move_num * value_size);
        r_w_file.read(reinterpret_cast<char *> (&valueType), value_size);
    }

    void ReadEle(const long &start_addr, ValueType &valueType) {
        r_w_file.open(file);

        r_w_file.seekg(start_addr);
//        std::cout << "\nPRE GOOD" << r_w_file.good() << '\n';
        r_w_file.read(reinterpret_cast<char *> (&valueType), value_size);
//        std::cout << "AFTER GOOD" << r_w_file.good() << '\n';

        r_w_file.close();

    }

    long WriteEle(const long &start_addr, const int &move_num, ValueType valueType) {
        r_w_file.seekp(start_addr + move_num * value_size);
        long addr = r_w_file.tellp();
        r_w_file.write(reinterpret_cast<char *> (&valueType), value_size);
        return addr;
    }

    long WriteEle(ValueType valueType) {
        r_w_file.open(file);

        r_w_file.seekp(0, std::ios::end);
//        r_w_file.seekg(0);
        long addr = r_w_file.tellp();
        r_w_file.write(reinterpret_cast<char *> (&valueType), value_size);

        r_w_file.close();

        return addr;
    }
};

#endif //TICKETSYSTEM_FILE_MANAGER_HPP
