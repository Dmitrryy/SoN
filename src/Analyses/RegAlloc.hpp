#pragma once

#include "Function.hpp"
#include "Liveness.hpp"
#include "Node.hpp"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>

namespace son {

class RegAlloc final {
public:
  struct AllocInfo final {
    size_t regIdxOrStackLockation = 0;
    std::pair<size_t, size_t> interval;
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

  AllocInfo info(Node *node) const { return m_regMap.at(node); }

  void dump(const Function::NamesMapTy &names) const {
    size_t max_live_n = 0;
    for (auto &&p : m_regMap) {
      max_live_n = std::max(max_live_n, p.second.interval.second);
    }

    std::vector<std::pair<Node *, AllocInfo>> info_s(m_regMap.begin(),
                                                     m_regMap.end());
    std::sort(info_s.begin(), info_s.end(), [](auto &&lhs, auto &&rhs) {
      return lhs.second.interval.first < rhs.second.interval.second;
    });

    std::cout << std::setw(3) << " " << '|';
    for (size_t i = 0; i < max_live_n; ++i) {
      std::cout << std::setw(3) << i << '|';
    }
    std::cout << std::endl;

    size_t counter = 0;
    for (auto &&info : info_s) {
      std::cout << std::setw(3) << counter++ << '|';
      for (size_t i = 0; i < max_live_n; ++i) {
        if (i >= info.second.interval.first &&
            i <= info.second.interval.second) {
          std::string str = "r";
          if (info.second.isSpill) {
            str = "s";
          }
          str += std::to_string(info.second.regIdxOrStackLockation);
          std::cout << std::setw(3) << str << '|';
        } else {
          std::cout << std::setw(3) << " " << '|';
        }
      }
      info.first->dump(std::cout, names);
      std::cout << std::endl;
    }
  }

private:
  void linearScanRegisterAllocation(const Function &F, const Liveness &LV) {
    auto &&liveNums = LV.getLiveNumbers();
    std::vector<std::pair<size_t, size_t>> intervals;
    std::vector<Node *> nodes;
    for (auto &&p : liveNums) {
      intervals.emplace_back(LV.liveInterval(p.first));
      nodes.emplace_back(p.first);
    }

    // sort interval indixes in order of increasing start point
    std::vector<size_t> interval_idx_s(intervals.size());
    std::iota(interval_idx_s.begin(), interval_idx_s.end(), 0);
    std::sort(interval_idx_s.begin(), interval_idx_s.end(),
              [&](auto &&lhs, auto &&rhs) {
                return intervals[lhs].first < intervals[rhs].first;
              });

    size_t stakFreeLocation = 0;
    std::deque<size_t> active;
    std::vector<size_t> free_regs(numRegs);
    std::iota(free_regs.rbegin(), free_regs.rend(), 0);

    // for each live interval i, in order of increasing start point
    for (auto i : interval_idx_s) {
      expireOldIntervals(i, stakFreeLocation, active, free_regs, intervals,
                         nodes);
      if (active.size() == numRegs) {
        spillAtInterval(i, stakFreeLocation, active, intervals, nodes);
      } else {
        // pop free register from pool
        auto fr = free_regs.back();
        free_regs.pop_back();

        m_regMap[nodes[i]] = AllocInfo{fr, intervals[i], false};

        // place i to active, sorted by increasing end point
        active.emplace_back(i);
        std::sort(active.begin(), active.end(), [&](auto &&lhs, auto &&rhs) {
          return intervals[lhs].second < intervals[rhs].second;
        });
      }
    }
  }

  void
  expireOldIntervals(size_t i, size_t &stakFreeLocation,
                     std::deque<size_t> &active, std::vector<size_t> &free_regs,
                     const std::vector<std::pair<size_t, size_t>> &intervals,
                     const std::vector<Node *> &nodes) {
    while (!active.empty() &&
           intervals[active.front()].second <= intervals[i].first) {
      // remove j from active
      size_t j = active.front();
      active.pop_front();

      // add register[j] to pool of free registers
      auto &&Info = m_regMap[nodes[j]];
      free_regs.push_back(Info.regIdxOrStackLockation);
    }
  }

  void spillAtInterval(size_t i, size_t &stakFreeLocation,
                       std::deque<size_t> &active,
                       const std::vector<std::pair<size_t, size_t>> &intervals,
                       const std::vector<Node *> &nodes) {
    size_t spill = active.back();
    if (intervals[spill].second > intervals[i].second) {
      auto &&iInfo = m_regMap[nodes[i]];
      auto &&spillInfo = m_regMap[nodes[spill]];
      assert(!spillInfo.isSpill);

      iInfo.regIdxOrStackLockation = spillInfo.regIdxOrStackLockation;
      iInfo.interval = intervals[i];

      // remove spill from active
      active.pop_back();
      // add i to active, sorted by increasing end point
      active.emplace_back(i);
      std::sort(active.begin(), active.end(), [&](auto &&lhs, auto &&rhs) {
        return intervals[lhs].second < intervals[rhs].second;
      });

      spillInfo.isSpill = true;
      spillInfo.regIdxOrStackLockation = stakFreeLocation;
    } else {
      // new stack location
      m_regMap[nodes[i]] = AllocInfo{stakFreeLocation, intervals[i], true};
    }
    ++stakFreeLocation;
  }
};

} // namespace son