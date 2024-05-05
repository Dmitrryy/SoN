#pragma once

#include "Function.hpp"
#include <Analyses/DomTree.hpp>

#include <unordered_map>
#include <vector>

namespace son {

class LoopInfo;

class Loop final {
  // header is front of vector
  std::vector<RegionNodeBase *> m_blocks;
  std::vector<RegionNodeBase *> m_latches;

  // tree of loops
  std::vector<Loop *> m_subLoops;
  Loop *m_parent = nullptr;

  friend LoopInfo;

protected:
  Loop() = default;
  Loop(RegionNodeBase *header) { m_blocks.push_back(header); }

public:
  const auto &blocks() const { return m_blocks; }
  const auto &latches() const { return m_latches; }
  Node *header() const { return m_blocks.front(); }
  Loop *parent() const { return m_parent; }
  // TODO
  Node *preheader() const;

  const auto &subloops() const { return m_subLoops; }
};

class LoopInfo final {
  std::vector<std::unique_ptr<Loop>> m_loops;
  std::vector<Loop *> m_topLoops;

  std::unordered_map<RegionNodeBase *, Loop *> m_BBMap;

public:
  Loop *getLoopFor(RegionNodeBase *region) { return m_BBMap[region]; }

  auto loopsCount() const { return m_loops.size(); }

  const auto &topLoops() const { return m_topLoops; }

  // build LoopInfo
  static LoopInfo loopAnalyze(const Function &F,
                              const Function::DFSResultTy &dfsRes,
                              const DomTree &DT) {
    auto &&[dfs, dfsParents, dfsNumbers, rpo] = dfsRes;
    LoopInfo LI;

    for (const auto &V : rpo) {
      std::vector<RegionNodeBase *> latches;
      for (const auto &P : V->predecessors()) {
        if (DT.dominates(V, P)) {
          latches.push_back(P);
        }
      }
      if (!latches.empty()) {
        LI.createLoop(V, latches);
      }
    }

    // fill loops body
    for (const auto &pL : LI.m_loops) {
      auto &&L = pL.get();
      auto *H = L->header();
      // traverse up CFG from latch to header
      auto workStack = L->m_latches;
      std::unordered_set<RegionNodeBase *> visited;
      while (!workStack.empty()) {
        auto *const V = workStack.back();
        workStack.pop_back();
        if (visited.count(V) || V == H) {
          continue;
        }
        visited.emplace(V);

        if (Loop *SubLoop = LI.getLoopFor(V)) {
          if (SubLoop->parent() == nullptr) {
            SubLoop->m_parent = L;
            L->m_subLoops.push_back(SubLoop);
          }
        } else {
          L->m_blocks.push_back(V);
          LI.m_BBMap[V] = L;
        }

        // insert predicessors
        for (auto &&p : V->predecessors()) {
          if (!visited.count(p) && p != H)
            workStack.emplace_back(p);
        }
      }
    }

    // find top loops
    for (const auto &pL : LI.m_loops) {
      auto &&L = pL.get();
      if (L->parent() == nullptr) {
        LI.m_topLoops.push_back(L);
      }
    }

    return LI;
  }

private:
  Loop *createLoop(RegionNodeBase *header,
                   std::vector<RegionNodeBase *> latches) {
    auto loop = new Loop(header);
    loop->m_latches = std::move(latches);
    m_loops.emplace_back(loop);

    assert(getLoopFor(header) == nullptr);
    m_BBMap[header] = loop;
    return loop;
  }
};

} // namespace son
