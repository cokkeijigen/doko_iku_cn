#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <filesystem>
#include "file_helper.h"
namespace fs = std::filesystem;
struct block
{
	char name[0x30];
	int32_t val1;
	int32_t val2;
	int32_t size; // or int64_t size
	int32_t val3;
};
FileHelper::ReadBuffer rb;
FileHelper::WriteBuffer wb;
fs::path curPath;

void dump(std::string path) {
	fs::path outPath(curPath);
	outPath /= path.substr(path.find_last_of("\\") + 1);
	if (rb.reader(path.c_str()).has_data()) {
		uint8_t* data = (uint8_t*)rb.data();
		size_t fileNum = (*(block*)data).size;
		block* blocks = (block*)(data + 0x40);
		uint8_t* start = (data + (fileNum * 0x40));
		size_t cur_pos = 0;
		for (size_t i = 0; i < fileNum - 1; i++)
		{
			block fileInfo = blocks[i];
			std::string fileName(fileInfo.name);
			size_t split = fileName.find_last_of("\\");
			fs::path out_path(outPath);
			if (split != std::string::npos) {
				out_path /= fileName.substr(0, split + 1);
				fileName.assign(fileName.substr(split + 1));
			}
			if (!fs::exists(out_path)) {
				fs::create_directories(out_path);
			}
			out_path /= fileName;
			wb.clear();
			wb.write((int8_t*)(start + cur_pos), fileInfo.size);
			fileName.assign(out_path.string());
			wb.out_file(fileName.c_str());
			cur_pos += fileInfo.size;
			std::cout<< fileInfo.name << std::endl;
		}
	}
}

int main(int argc, char* argv[]) {
	std::string path(argv[0]);
	curPath.assign(path.substr(0, path.find_last_of("\\") + 1));
	if (argc == 2) {
		dump(argv[1]);
		std::cout<< std::endl;
	}
	else {
		std::cout<< "Illegal parameter!!!" << std::endl;
	}
	system("pause");
	return 0;
}