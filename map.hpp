////////////////////////////////////////////////////////////////////////////////
//
// file mapping map class.
//
// Maps readable and writable files
//
////////////////////////////////////////////////////////////////////////////////


#ifndef _MAP_HPP_INCLUDED_
#define _MAP_HPP_INCLUDED_


#ifdef _MSC_VER
  #include <windows.h>
#else
  #include <sys/mman.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
#endif

class map {
public:
  map(const char *filename, const char *mode, size_t length=0) {
    construct(filename, mode, length);
  }

  map(const std::string &filename, const char *mode, size_t length=0) {
    construct(filename.c_str(), mode, length);
  }

  map(map &&rhs) { move(rhs); }
  void operator=(map &&rhs) { move(rhs); }
  ~map() { destroy(); }
  uint8_t *data() const { return (uint8_t*)data_; }
  size_t size() const { return size_; }
  uint8_t *begin() const { return (uint8_t*)data_; }
  uint8_t *end() const { return (uint8_t*)data_ + size_; }
  void truncate(size_t size) { do_truncate(size); }

private:
  void construct(const char *filename, const char *mode, size_t size=0) {
    while (*mode) {
      switch (*mode++) {
        case 'r': read_ = true; break;
        case 'w': write_ = true; break;
        default: return;
      }
    }

    #ifdef _MSC_VER
      if (read_ && write_) {
      } else if (read_) {
        file_ = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (file_ != INVALID_HANDLE_VALUE) {
          LARGE_INTEGER size = {0};
          ::GetFileSizeEx(file_, &size);
          size_ = size_t(size.QuadPart);
          map_ = ::CreateFileMappingW(file_, NULL, PAGE_READONLY, 0, 0, NULL);
          if (map_ != NULL) {
            data_ = ::MapViewOfFile(map_, FILE_MAP_READ, 0, 0, (SIZE_T)size_);
          } else {
            destroy();
          }
        }
      } else if (write_) {
        file_ = ::CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
        if (file_ != INVALID_HANDLE_VALUE) {
          truncate(size);
          map_ = ::CreateFileMappingW(file_, NULL, PAGE_READWRITE, 0, 0, NULL);
          if (map_ != NULL) {
            data_ = ::MapViewOfFile(map_, FILE_MAP_READ, 0, 0, (SIZE_T)size_);
          } else {
            destroy();
          }
        }
      }
    #else
      if (read_ && write_) {
      } else if (read_) {
        fd_ = open(filename, O_RDONLY, 0);
        if (fd_ != -1) {
          size_ = size_t(lseek(fd_, 0l, SEEK_END));
          lseek(fd_, 0l, SEEK_SET);
          data_  = mmap(NULL, size_, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd_, 0);
        }
      } else if (write_) {
        printf("writing %s %ld\n", filename, long(size));
        fd_ = open(filename, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
        printf("fd=%d\n", fd_);
        if (fd_ != -1) {
          truncate(size);
          data_  = mmap(NULL, size_, PROT_WRITE, MAP_SHARED, fd_, 0);
        }
      }
    #endif
  }

  void move(map &rhs) {
    size_ = rhs.size_;
    data_ = rhs.data_;
    #ifdef _MSC_VER
      file_ = rhs.file_;
      rhs.file_ = INVALID_HANDLE_VALUE;
      map_ = rhs.map_;
      rhs.map_ = INVALID_HANDLE_VALUE;
    #else
      fd_ = rhs.fd_;
      rhs.fd_ = -1;
    #endif
    rhs.size_ = 0;
    rhs.data_ = nullptr;
  }

  void destroy() {
    unmap();
    #ifdef _MSC_VER
      if (file_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_);
        file_ = INVALID_HANDLE_VALUE;
      }
    #else
      if (fd_) {
        close(fd_);
        fd_ = -1;
      }
    #endif
  }

  void unmap() {
    if (data_) {
      #ifdef _MSC_VER
        if (map_ != NULL) CloseHandle(map_);
        if (data_) UnmapViewOfFile(data_);
      #else
        munmap(data_, size_);
      #endif
    }
    data_ = nullptr;
    size_ = 0;
  }

  void do_truncate(size_t size) {
    #ifdef _MSC_VER
      if (write_) {
        LARGE_INTEGER sz;
        sz.QuadPart = size;
        if (
          SetFilePointerEx(file_, sz, NULL, FILE_BEGIN) &&
          SetEndOfFile(file_)
        ) {
          size_ = size;
        }
      }
    #else
      if (write_ && ftruncate(fd_, size) != -1) {
        size_ = size;
      }
    #endif
  }

  #ifdef _MSC_VER
    HANDLE file_ = INVALID_HANDLE_VALUE;
    HANDLE map_ = INVALID_HANDLE_VALUE;
  #else
    int fd_ = -1;
  #endif
  void *data_ = nullptr;
  size_t size_;
  bool read_ = false;
  bool write_ = false;
};

#endif

