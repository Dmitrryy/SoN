#pragma once

#include <Analyses/DomTree.hpp>
#include <Analyses/Loop.hpp>
#include <Function.hpp>

#include <unordered_map>

namespace son {

class Liveness final {
public:
  using LiveNumbers = std::unordered_map<Node *, size_t>;

private:
  LiveNumbers m_liveNumbers;

public:
  Liveness() = default;
  Liveness(const Function &F) { recalc(F); }

  void recalc(const Function &F) {
    DomTree DT(F);
    auto &&DataMap = F.dataSchedule(DT);
    auto &&dfsResult = F.dfs();
    auto &&LI = LoopInfo::loopAnalyze(F, dfsResult, DT);
    auto &&linRegs = F.linearize(DT, dfsResult, LI);

    // live numbers
    m_liveNumbers = _liveNumbers(linRegs, DataMap);

    // intervals
    // TODO
  }

  auto liveNumber(Node *node) const { return m_liveNumbers.at(node); }

private:
  static LiveNumbers _liveNumbers(const std::vector<RegionNodeBase *> &linOrder,
                                  const Function::DataMapperTy &rangeToInstrs) {
    LiveNumbers res;
    size_t curLiveNum = 0;
    for (auto R : linOrder) {
      if (!rangeToInstrs.contains(R)) {
        continue;
      }

      // phi
      for (auto Phi : R->phis()) {
        res[Phi] = curLiveNum;
      }

      for (auto I : rangeToInstrs.at(R)) {
        curLiveNum += 2;
        res[I] = curLiveNum;
      }

      curLiveNum += 2;
    }

    return res;
  }
};

} // namespace son