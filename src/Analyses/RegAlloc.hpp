#pragma once

#include "Function.hpp"
#include "Liveness.hpp"
#include "Node.hpp"

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace son {

class RegAlloc final {
public:
  struct AllocInfo final {
    size_t regIdxOrStackLockation = 0;
    bool isSpill = false;
  };

private:
  // map value to register id
  std::unordered_map<Node *, AllocInfo> m_regMap;
  size_t numRegs = 0;

public:
  RegAlloc() = default;
  RegAlloc(const Function &F, size_t numRegs, const Liveness &LV)
      : numRegs(numRegs) {
    linearScanRegisterAllocation(F, LV);
  }

  void linearScanRegisterAllocation(const Function &F, const Liveness &LV) {
    auto &&liveNums = LV.getLiveNumbers();
    std::vector<std::pair<size_t, size_t>> intervals;
    std::vector<Node *> nodes;
    for (auto &&p : liveNums) {
      intervals.emplace_back(LV.liveInterval(p.first));
      nodes.emplace_back(p.first);
    }
    std::sort(intervals.begin(), intervals.end(),
              [](auto &&lhs, auto &&rhs) { return lhs.first < rhs.first; });

    std::vector<size_t> active;
    std::vector<size_t> free_regs(numRegs);
    std::iota(free_regs.rbegin(), free_regs.rend(), 0);
    // for each live interval i, in order of increasing start point
    for (size_t i = 0; i < intervals.size(); ++i) {
      expireOldIntervals(i, active);
      if (active.size() == numRegs) {
        spillAtInterval(i);
      } else {
        // pop free register from pool
        auto fr = free_regs.back();
        free_regs.pop_back();

        m_regMap[nodes[i]] = AllocInfo{fr, false};

        // place i to active, sorted by increasing end point
        active.emplace_back();
        std::push_heap(active.begin(), active.end(),
                       [&intervals](auto &&lhs, auto &&rhs) {
                         return intervals[lhs].second < intervals[rhs].second;
                       });
      }
    }
  }

  void expireOldIntervals(size_t i, std::vector<size_t> &active) {
    // TODO
  }

  void spillAtInterval(size_t i) {
    // TODO
  }
};

} // namespace son