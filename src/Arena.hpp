#pragma once

#include <array>
#include <memory>
#include <vector>
#include <list>

namespace son {

template <size_t Size> class Arena final {
  std::array<uint8_t, Size> m_data;
  size_t m_end = 0;
  uint8_t *m_LastPtr = 0;

public:
    size_t getSize() const { return Size; }
    size_t getFreeSpace() const { return Size - m_end; }

  void *Allocate(size_t size) {
    if (Size - m_end >= size) {
      m_LastPtr = &m_data[m_end];
      m_end += size;
      return m_LastPtr;
    }
    return nullptr;
  }

  void Deallocate(void *ptr) {
    if (ptr == m_LastPtr) {
      m_end = m_LastPtr - &m_data[0];
    }
  }
};

class Allocator final {
  std::list<Arena<10000>> m_data;

public:
    Allocator() {
        NewArea();
    }

    void *Allocate(size_t size) {
        auto *res = m_data.back().Allocate(size);
        if (!res) {
            NewArea();
            res = m_data.back().Allocate(size);
        }
        return res;
    }

    void Deallocate(void *ptr) {
        m_data.back().Deallocate(ptr);
    }

    void NewArea() {
        m_data.emplace_back();
    }

    void clear() { m_data.clear(); }
};

} // namespace son