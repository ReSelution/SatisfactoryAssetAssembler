#include <memory_resource>

template<size_t BufferSize = 16 * 1024>
class ThreadArena {
  struct Storage {
    char buffer[BufferSize];
    std::pmr::monotonic_buffer_resource pool;
    Storage() : buffer{}, pool(buffer, BufferSize) {}
  };

  static inline thread_local Storage storage;

public:
  template<typename T, typename... Args>
  static T *make(Args &&...args) {
    void *mem = storage.pool.allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  static void reset() { storage.pool.release(); }
};
