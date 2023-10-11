#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <stack>

namespace work {

    typedef struct {
        char name[0x30];
        int32_t val1; // offset?
        int32_t val2; // offset?
        int32_t size; // or int64_t size?
        int32_t val3;
    } entry;

    const char* name_head = "head.bin";
    const char* name_data = "data.bin";
    namespace fsystem = std::filesystem;
    typedef fsystem::directory_iterator diriter;
    // mode std::ios::binary | std::ios::out
    std::fstream head(name_head, 0x20 | 0x02);
    std::fstream data(name_data, 0x20 | 0x02);
    work::entry top{ "DATA$TOP", 0, 0, 1, 0 };
    std::stack<fsystem::path> directories;
    std::string progm_dir_path;
    std::string output_file_path;
    size_t input_dir_path_length;
    size_t current_offset = 0;
    bool is_warking = false;

    void init(fsystem::path pathWk, std::string pathIpt) {
        if (fsystem::is_directory(pathIpt)) {
            directories.push(pathIpt);
            input_dir_path_length = pathIpt.size() + 1;
            output_file_path.assign(pathIpt.append(".pak"));
            progm_dir_path.assign(pathWk.parent_path().string());
            head.write((const char*)&top, sizeof(entry));
            work::is_warking = true;
        }
        else {
            work::is_warking = false;
        }
    }

    void clear() {
        if (work::data.is_open()) work::data.close();
        if (work::head.is_open()) work::head.close();
        if (fsystem::exists(name_data)) std::remove(name_data);
        if (fsystem::exists(name_head)) std::remove(name_head);
    }

    void pack_completing() {
        printf("> PACK COMPLETING...\n");
        system("@echo off");
        // copy /b head.bin+data.bin newpack.pak
        std::string cmd("copy /b ");
        cmd.append(name_head);
        cmd.append("+");
        cmd.append(name_data);
        cmd.append(" ");
        cmd.append(output_file_path);
        system(cmd.c_str());
        work::clear();
    }

    size_t write_data(std::fstream& ifs) {
        work::data << ifs.rdbuf();
        ifs.seekg(0, std::ios::end);
        return ifs.tellg();
    }

    void write_head(work::entry& info) {
        head.seekg(0, std::ios::beg);
        head.write((const char*)&top, sizeof(entry));
        head.seekg(0, std::ios::end);
        head.write((const char*)&info, sizeof(entry));
    }

    size_t add_file_to_pack(fsystem::path dir, size_t offset) {
        work::directories.pop();
        size_t length = input_dir_path_length;
        for (const auto& e : diriter(dir)) {
            if (e.is_directory()) {
                directories.push(e.path());
            }
            else if (e.is_regular_file()) {
                std::string file(e.path().string());
                file.assign(file.substr(length));
                if (file.size() >= 0x30) continue;
                std::cout << file << "\t: ";
                ++top.size; // pack entry count
                std::fstream tmp(e.path(), 0x23);
                work::entry info{ 0, 0, 0, 0, 0 };
                strcpy(info.name, file.c_str());
                info.size = write_data(tmp);
                info.val1 = offset;
                info.val2 = offset;
                offset += info.size;
                write_head(info);
                head.flush();
                data.flush();
                tmp.close();
                std::cout << "ok" << std::endl;
            }
        }
        return offset;
    }

    void run_starting() {
        if (work::is_warking) {
            try {
                while (!directories.empty()) {
                    current_offset = add_file_to_pack(directories.top(), current_offset);
                }
            } catch(std::exception &e) {
                std::cout<< "err: " << e.what() << std::endl;
            }
            work::pack_completing();
            std::cout << "> DONE. FILES: " << top.size - 1 << std::endl;
        }
        else {
            std::cout << "> FILE PACKAGING FAILED!" << std::endl;
        }
    }
    
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        SetConsoleOutputCP(65001);
        work::init(argv[0], argv[1]);
        work::run_starting();
        std::cin.ignore();
    }
    else {
        work::clear();
    }
	return 0;
}