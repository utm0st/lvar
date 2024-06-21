#include "lvar_obj.h"

#include <iostream>
#include <errno.h>              // errno
#include <fcntl.h>              // open
#include <sys/stat.h>           // fstat
#include <sys/mman.h>           // mmap, munmap
#include <cstdio>               // sscanf
#include <unistd.h>             // close
#include <string.h>             // strerror

namespace lvar {
  namespace obj {

    // avoid fkin leaks
    class raiifile final {
    public:
      raiifile(char const* filepath)
      {
        err = false;
        fd = open(filepath, O_RDONLY);
        if(fd == -1) {
          std::cerr << "couldn't open file " << filepath << '\n';
          err = true;
          return;
        }
        // get size of file
        struct stat sb;
        if(fstat(fd, &sb) == -1) {
          std::cerr << "fstat : couldn't get file size: " << strerror(errno) << '\n';
          err = true;
          return;
        }
        sz = sb.st_size;
        // map file into mem
        data = static_cast<char*>(mmap(nullptr, sz, PROT_READ, MAP_SHARED, fd, 0));
        if(data == MAP_FAILED) {
          std::cerr << "mmap : error mapping file: " << strerror(errno) << '\n';
          err = true;
          return;
        }
      }
      ~raiifile()
      {
        munmap(data, sz);
        close(fd);
      }
      auto ptr() const noexcept { return data; }
      auto size() const noexcept { return sz; }
      auto error() const noexcept{ return err; }
    private:
      char* data;
      std::size_t sz;
      int fd;
      bool err;
    };

    bool parse_file(char const* filepath, mesh& o)
    {
      raiifile file(filepath);
      if(file.error()) {
        return false;
      }
      // count lines
      unsigned int ln{ 0 };
      char* ptr{ file.ptr() };
      for(std::size_t i{ 0 }; i < file.size(); ++i) {
        if(ptr[i] == '\n') {
          ++ln;
        }
      }
      // do rough estimation of vertices and faces to reserve space, yolo ~60% faces, ~40% vertices
      o.vertices.reserve(static_cast<unsigned int>(ln * 0.4));
      o.faces.reserve(static_cast<unsigned int>(ln * 0.6));
      o.indices.reserve(o.faces.capacity() * 3);
      // parse
      char* curr{ file.ptr() };
      char const* end{ curr + file.size() };
      while(curr < end) {
        if(*curr == 'v') {
          // vertex
          vertex v;
          if(std::sscanf(curr, "v %f %f %f", &v.x, &v.y, &v.z) != 3) {
            std::cerr << __FUNCTION__ << ": couldn't get vertex data\n";
            return false;
          }
          o.vertices.emplace_back(v);
        } else if(*curr == 'f') {
          // face
          face f;
          if(std::sscanf(curr, "f %u %u %u", &f.idx_x, &f.idx_y, &f.idx_z) != 3) {
            std::cerr << __FUNCTION__ << ": couldn't get face data\n";
            return false;
          }
          o.faces.emplace_back(f);
          o.indices.emplace_back(f.idx_x);
          o.indices.emplace_back(f.idx_y);
          o.indices.emplace_back(f.idx_z);
        }
        // now move to end of line
        while(curr < end && *curr != '\n') {
          ++curr;
        }
        if(curr < end) {
          // skip newline char
          ++curr;
        }
      }
      return true;
    }

  };
};
