#pragma once

#include "Function.hpp"

namespace son {

class DomTree final {
  std::unordered_map<RegionNodeBase *, size_t> m_nodeToIdx;
  std::vector<RegionNodeBase *> m_idxToNode;
  std::vector<size_t> m_idomTree;

public:
  DomTree() = default;
  DomTree(const Function &F) { recalc(F); }

  void recalc(const Function &F) {
    auto &&dfsResult = F.dfs();
    auto &&semi = F.semiDominators(dfsResult);

    m_idomTree = F.iDominators(semi);
    std::tie(m_idxToNode, std::ignore, m_nodeToIdx, std::ignore) = dfsResult;
  }

  bool dominates(RegionNodeBase *A, RegionNodeBase *B) const {
    return dominates(m_nodeToIdx.at(A), m_nodeToIdx.at(B));
  }

  RegionNodeBase *idom(RegionNodeBase *A) const {
    return m_idxToNode[m_idomTree[m_nodeToIdx.at(A)]];
  }

private:
  bool dominates(size_t aIdx, size_t bIdx) const {
    // go up to root
    while (bIdx != 0 && bIdx != aIdx) {
      bIdx = m_idomTree[bIdx];
    }
    return aIdx == bIdx;
  }
};

} // namespace son