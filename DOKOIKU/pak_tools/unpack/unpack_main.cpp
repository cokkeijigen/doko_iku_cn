#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <filesystem>
#include <time.h>
#include "file_helper.h"

namespace work {

	typedef struct {
		char name[0x30];
		int32_t val1; // offset?
		int32_t val2; // offset?
		int32_t size; // or int64_t size?
		int32_t val3;
	} entry;

	files::readbuffer  rb;
	files::writebuffer wb;
	std::filesystem::path wkpath;

	void make_dirs(const std::string& path) {
		if (std::filesystem::exists(path)) return;
		std::filesystem::create_directories(path);
	}

	std::string dump(std::string& path) {
		if (!rb.load(path.c_str())) return "> PACK LOADING FAILED OR PACK IS EMPTY!";
		if (memcmp(rb.read(0, 8), "DATA$TOP", 8)) return "> CAN NOT READ THIS PACK!";
		uint8_t* data = (uint8_t*)rb.read().data();
		entry* entrys = (entry*)data;
		size_t fileNum = entrys->size;
		uint8_t* start = (data + (fileNum * 0x40));
		for (size_t i = 1; i < fileNum; i++) {
			entry* info = &entrys[i];
			std::string file(info->name);
			std::filesystem::path otpath(wkpath/file);
			std::string wtpath(otpath.parent_path().string());
			wb.clear().write(start + info->val1, info->size);
			work::make_dirs(wtpath);
			wtpath.assign(otpath.string());
			wb.save(wtpath.c_str());
			std::cout << file << std::endl;
		}
		return "> UNPACK DONE.";
	}
}

int main(int argc, char* argv[]) {
	if (argc == 2) {
		std::string path(argv[1]);
		work::wkpath.assign(argv[0]);
		work::wkpath.assign(work::wkpath.parent_path());
		work::wkpath /= path.substr(path.find_last_of("\\") + 1);
		std::cout << work::dump(path) << std::endl;
		work::rb.clear();
		work::wb.clear();
	}
	else {
		std::cout << "> ILLEGAL PARAMETER!" << std::endl;
	}
	std::cin.ignore();
	return 0;
}