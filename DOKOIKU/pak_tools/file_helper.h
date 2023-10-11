#pragma once

namespace files {

    class readbuffer {
        FILE * file_ptr;
        uint8_t* buffer;
        uint8_t* tmpbuf;
        size_t   bfsize;
        bool   is_clear;

    public:
        readbuffer(const char* filepath): readbuffer() { read(filepath); }
        readbuffer(): buffer(nullptr), file_ptr(nullptr), tmpbuf(nullptr), bfsize(0), is_clear(true){ }
        
        size_t load(const char* filepath) {
            if (file_ptr = fopen(filepath, "rb")) {
                fseek(file_ptr, 0, SEEK_END);
                bfsize = ftell(file_ptr);
            }
            return bfsize;
        }

        uint8_t* read(size_t pos, size_t size) {
            if (tmpbuf != nullptr) delete[] buffer;
            if (size > 0) tmpbuf = new uint8_t[size];
            if (pos >= bfsize) return NULL;
            if (file_ptr != nullptr && size > 0) {
                fseek(file_ptr, 0, pos);
                fread(tmpbuf, 1, size, file_ptr);
            }
            else if (buffer && size > 0) {
                size_t copy = bfsize - pos;
                copy = size > copy ? (size - copy) : size;
                memcpy(tmpbuf, buffer, size);
            }
            return tmpbuf;
        }

        readbuffer& read(const char* filepath = nullptr) {
            if (filepath == nullptr && file_ptr && is_clear) {
                if (buffer) delete[] buffer;
                buffer = new uint8_t[bfsize];
                fseek(file_ptr, 0, SEEK_SET);
                fread(buffer, 1, bfsize, file_ptr);
                fclose(this->file_ptr);
                this->file_ptr = nullptr;
                this->is_clear = false;
            }
            else if (filepath != nullptr) {
                if (!this->is_clear) this->clear();
                FILE* file = fopen(filepath, "rb");
                if (file != NULL) {
                    fseek(file, 0, SEEK_END);
                    bfsize = ftell(file);
                    buffer = new uint8_t[bfsize];
                    fseek(file, 0, SEEK_SET);
                    fread(buffer, 1, bfsize, file);
                    this->is_clear = false;
                    fclose(file);
                }
            }
            return *this;
        }

        size_t size() {
            return this->bfsize;
        }

        bool empty() {
            return !this->bfsize;
        }

        uint8_t* data() {
            return this->buffer;
        }

        readbuffer& clear() {
            if (buffer) delete[] buffer;
            if (tmpbuf) delete[] tmpbuf;
            if (file_ptr) fclose(file_ptr);
            this->is_clear = true;
            this->file_ptr = nullptr;
            this->buffer = nullptr;
            this->tmpbuf = nullptr;
            this->bfsize = 0x00;
            return *this;
        }

        ~readbuffer() { clear(); }
    };

    class writebuffer {
        const char* tmpname;
        uint8_t* buffer;
        size_t bfsize;
        size_t length;
        size_t exsize;
        size_t initsz;
        FILE *tmpfptr;
    protected:
        void initialization() {
            if (buffer) delete[] buffer;
            if (tmpfptr) fclose(tmpfptr);
            if (tmpname) remove(tmpname);
            tmpname = nullptr;
            tmpfptr = nullptr;
            buffer = new uint8_t[initsz];
            bfsize = initsz;
            length = 0x00;
        }

        void expandbuffer(size_t bfsz) {
            if (bfsz == NULL) {
                bfsize = bfsize + exsize;
            }
            else {
                size_t bfnsz = length + bfsz;
                if (bfnsz < bfsize) return;
                while (bfnsz >= (bfsize += exsize));
            }
            uint8_t* nbuf = new uint8_t[bfsize];
            if (length && buffer) { 
                memcpy(nbuf, buffer, length); 
                delete[] buffer;
            }
            buffer = nbuf;
        }

    public:
        writebuffer(): writebuffer(1024, 500) {}
        writebuffer(size_t initsz, size_t exsize): initsz(initsz), exsize(exsize), tmpname(nullptr), tmpfptr(nullptr){
            this->initialization();
        }

        writebuffer& write(const uint8_t* buf, size_t bfsz) {
            if (!tmpname) {
                this->expandbuffer(bfsz);
                memcpy(&buffer[length], buf, bfsz);
                length = length + bfsz;
            }
            else {
                if(!tmpfptr) tmpfptr = fopen(tmpname, "wb");
                fwrite(buf, 1, bfsz, tmpfptr);
            }
            return *this;
        }

        writebuffer& to_tmp_file(const char* filename) {
            this->tmpname = filename;
        }

        uint8_t* data() {
            return tmpname && tmpfptr ? nullptr : this->buffer;
        }

        size_t size() {
            if (tmpname && tmpfptr) {
                size_t curpos = ftell(tmpfptr);
                fseek(tmpfptr, 0, SEEK_END);
                size_t size = ftell(tmpfptr);
                fseek(tmpfptr, 0, curpos);
                return size;
            }
            return this->length;
        }

        bool save(const char* path) {
            if (tmpname) {
               fclose(tmpfptr);
               tmpfptr = nullptr;
               if (path) return rename(tmpname, path);
               tmpname = nullptr;
            }
            else {
                FILE* outfp = fopen(path, "wb");
                if (outfp == NULL) return false;
                fwrite(buffer, 1, length, outfp);
                fclose(outfp);
            }
            return true;
        }

        writebuffer& clear() { 
            this->initialization(); 
            return *this;
        }
    };
}