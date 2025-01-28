#pragma once

namespace Utils {

    class PackManger {
        struct Entry {
            char name[0x30];
            int32_t offset;
            int32_t value1;
            int32_t length;
            int32_t value2;
        };
    private:
        size_t size{ NULL };
        Entry* entrys{ NULL };
        FILE* file{ NULL };

        inline static void ToLowerCase(char* str) {
            while (*str) {
                if (*str >= 'A' && *str <= 'Z') {
                    *str = *str + 0x20;
                }
                str++;
            }
        }

        void Clear() {
            if (this->file) std::fclose(this->file);
            if (this->entrys) delete[] this->entrys;
            this->entrys = NULL;
            this->file = NULL;
            this->size = NULL;
        }

        inline void Init(const char* pak, Entry head = {}) {
            if (this->file = std::fopen(pak, "rb"); !this->file) {
                return;
            }
            std::fseek(this->file, 0, SEEK_END);
            this->size = std::ftell(this->file);
            if (this->size < 0x40) return;
            std::rewind(this->file);
            if (std::fread(&head, sizeof(Entry), 1, this->file) != 1) {
                this->Clear();
                return;
            }
            if (memcmp(head.name, "DATA$TOP", 8)) {
                this->Clear();
                return;
            }
            std::rewind(this->file);
            this->entrys = new Entry[head.length]{};
            if (std::fread(this->entrys, sizeof(Entry), head.length, this->file) != head.length) {
                this->Clear();
                return;
            }
            if (this->entrys) for (size_t i = 1; i < this->entrys->length; i++) {
                PackManger::ToLowerCase(this->entrys[i].name);
            }
        }

    public:
        PackManger(const char* pak) {  this->Init(pak); }
        ~PackManger() { this->Clear(); }

        inline bool HasData() const {
            return this->size && this->entrys && this->file;
        }

        int GetFileIndex(std::string name) {
            if (this->HasData()) for (size_t i = 1; i < this->entrys->length; i++) {
                PackManger::ToLowerCase(const_cast<char*>(name.c_str()));
                if (name == this->entrys[i].name) {
                    return i;
                }
            }
            return -1;
        }

        int GetFileSize(int index) const {
            if (index < 0 || index >= this->entrys->length) {
                return -1;
            }
            return this->entrys[index].length;
        }

        bool GetFileData(int index, void* buffer) {
            if (index < 0 || index >= this->entrys->length) {
                return false;
            }
            auto&& entry = this->entrys[index];
            std::fseek(this->file, entry.offset + (this->entrys->length * 0x40), SEEK_SET);
            return entry.length == std::fread(buffer, 1, entry.length, this->file);
        }

    };

}