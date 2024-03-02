#pragma once

#include "Node.hpp"
#include <Analyses/DomTree.hpp>
#include <Analyses/Loop.hpp>
#include <Function.hpp>

#include <iterator>
#include <unordered_map>

namespace son {

class Liveness final {
public:
  using LiveNumbers = std::unordered_map<Node *, size_t>;

private:
  LiveNumbers m_liveNumbers;
  std::unordered_map<RegionNodeBase *, std::vector<Node *>> m_liveIn;
  std::unordered_map<Node *, std::pair<size_t, size_t>> m_intervals;

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

    // traverse blocks in reversed linear order
    for (auto &&RIt = linRegs.rbegin(), RItEnd = linRegs.rend(); RIt != RItEnd;
         ++RIt) {
      auto R = *RIt;
      std::unordered_set<Node *> live;
      const auto &&RSuccessors = R->successors();

      // calc initial liveset for R:
      //   union of livesets in succ blocks
      for (auto S : RSuccessors) {
        auto &&SLive = m_liveIn[S];
        std::copy(SLive.begin(), SLive.end(), std::inserter(live, live.end()));
      }
      //   + succs phi real inputs
      for (auto S : RSuccessors) {
        for (auto Phi : S->phis()) {
          // input of R
          live.emplace(Phi->getValOf(R));
        }
      }

      // append liverange of current BB to inst
      const auto BBInterval = m_intervals[R];
      for (auto ins : live) {
        m_intervals[ins] = {
            std::min(BBInterval.first, m_intervals[ins].first),
            std::max(BBInterval.second, m_intervals[ins].second)};
      }

      // reverse iterate block insts (non-phi)
      auto &&RInstrs = DataMap[R];
      for (auto &&InstIt = RInstrs.rbegin(), ItE = RInstrs.rend();
           InstIt != ItE; ++InstIt) {
        // shorten the inst live interval
        m_intervals[*InstIt].first = m_liveNumbers[*InstIt];
        // min live interval is [ln, ln + 2)
        if (m_intervals[*InstIt].second < m_intervals[*InstIt].first + 2) {
          m_intervals[*InstIt].second = m_intervals[*InstIt].first + 2;
        }
        if (live.contains(*InstIt)) {
          live.erase(*InstIt);
        }

        // iterate inputs
        for (auto Input : (*InstIt)->operands()) {
          m_intervals[Input] = {
              std::min(BBInterval.first, m_intervals[Input].first),
              std::max(m_liveNumbers[*InstIt], m_intervals[Input].second)};
          live.emplace(Input);
        }
      }

      // remove phis from current liveset
      for (auto Phi : R->phis()) {
        if (live.contains(Phi)) {
          live.erase(Phi);
        }
        // NOTE: It is skipped in original algorithm?
        m_intervals[Phi].first = m_liveNumbers[Phi];
      }

      // if block is loop header
      auto *RL = LI.getLoopFor(R);
      if (RL != nullptr && RL->header() == R) {
        // find loop end
        size_t loopEnd = 0;
        for (auto B : RL->blocks()) {
          loopEnd = std::max(loopEnd, m_intervals[B].second);
        }
        // add life range of [loop_header.begin, loop_end)
        for (auto opd : live) {
          m_intervals[opd] = {
              std::min(BBInterval.first, m_intervals[opd].first),
              std::max(loopEnd, m_intervals[opd].second)};
        }
      }

      m_liveIn[R] = std::vector(live.begin(), live.end());
    }
  }

  auto liveNumber(Node *node) const { return m_liveNumbers.at(node); }
  auto liveInterval(Node *node) const { return m_intervals.at(node); }
  auto &liveIn(RegionNodeBase *region) const & { return m_liveIn.at(region); }

private:
  LiveNumbers _liveNumbers(const std::vector<RegionNodeBase *> &linOrder,
                           const Function::DataMapperTy &rangeToInstrs) {
    LiveNumbers res;
    size_t curLiveNum = 0;
    for (auto R : linOrder) {
      auto liveFrom = curLiveNum;
      if (!rangeToInstrs.contains(R)) {
        m_intervals[R] = {liveFrom, liveFrom};
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
      auto liveTo = curLiveNum;

      m_intervals[R] = {liveFrom, liveTo};
    }

    return res;
  }
};

} // namespace son