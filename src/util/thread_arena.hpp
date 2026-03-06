#pragma once
#include <memory_resource>

class ThreadArena {
  static constexpr size_t BufferSize = 8 * 1024;
public:
  struct Storage {
    char buffer[BufferSize];
    std::pmr::monotonic_buffer_resource pool;
    Storage(): buffer{}, pool(buffer, BufferSize) {};
  };
private:

  static Storage &get_storage() {
    thread_local Storage storage;
    return storage;
  }

public:
  template<typename T, typename... Args>
  static T *make(Args &&...args) {
    void *mem = get_storage().pool.allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  static void reset() { get_storage().pool.release(); }
};