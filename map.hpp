////////////////////////////////////////////////////////////////////////////////
//
// file mapping map class.
//
// Maps readable and writable files
//
////////////////////////////////////////////////////////////////////////////////


#ifndef _MAP_HPP_INCLUDED_
#define _MAP_HPP_INCLUDED_


#ifndef _MSC_VER
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

  void truncate(size_t size) {
    printf("truncating %08lx\n", (long)size);
    if (write_ && ftruncate(fd_, size) != -1) {
      printf("truncated %08lx\n", (long)size);
      size_ = size;
    }
  }

private:

  void construct(const char *filename, const char *mode, size_t size=0) {
    while (*mode) {
      switch (*mode++) {
        case 'r': read_ = true; break;
        case 'w': write_ = true; break;
        default: return;
      }
    }

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
        /*lseek(fd_, size-1, SEEK_SET);
        (void)write(fd_, (void*)"", 1);
        lseek(fd_, 0, SEEK_SET);*/
        data_  = mmap(NULL, size_, PROT_WRITE, MAP_SHARED, fd_, 0);
      }
    }
  }

  void move(map &rhs) {
    fd_ = rhs.fd_;
    size_ = rhs.size_;
    data_ = rhs.data_;
    rhs.fd_ = -1;
    rhs.size_ = 0;
    rhs.data_ = nullptr;
  }

  void destroy() {
    unmap();
    if (fd_) {
      close(fd_);
      fd_ = -1;
    }
  }

  void unmap() {
    if (data_) {
      munmap(data_, size_);
    }
    data_ = nullptr;
    size_ = 0;
  }

  int fd_ = -1;
  void *data_ = nullptr;
  size_t size_;
  bool read_ = false;
  bool write_ = false;
};

#endif

