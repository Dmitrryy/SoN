#include "Function.hpp"
#include "InstVisitor.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace son {

namespace {
struct GraphChecker : public InstVisitor<GraphChecker> {
  bool isValid = true;

  void expected(bool cond, const std::string &msg) {
    if (!cond) {
      isValid = false;
      std::cerr << msg << std::endl;
    }
  }

  void visitRegionNode(RegionNode &node) {
    for (auto &&p : node.predicessors()) {
      expected(isa<RegionNode, StartNode>(p),
               "[Region] Unexpected Region predicessor: " +
                   getOpcName(p->nodeTy()));
    }

    for (auto &&s : node.operands()) {
      expected(
          isa<JmpNode, IfTrueNode, IfFalseNode, RetNode>(s),
          "[Region] Unexpected Region input: " + getOpcName(s->nodeTy()));
    }

    for (auto &&s : node.users()) {
      expected(
          isa<JmpNode, IfNode, PhiNode, RetNode>(s),
          "[Region] Unexpected Region users " + getOpcName(s->nodeTy()));
    }

    for (auto &&p : node.successors()) {
      expected(isa<RegionNode, EndNode>(p),
               "[Region] Unexpected Region successor: " +
                   getOpcName(p->nodeTy()));
    }
  }

  void visitIfNode(IfNode &node) {
    if (node.opCount() != 2) {
      std::cout << "[IfNode] Invalid operand count(!= 2): " << node.opCount()
                << std::endl;
      isValid = false;
    }

    if (node.getCondition()->valueTy() != ValueType::Int1) {
      std::cout << "[IfNode] Invalid condition type" << std::endl;
      isValid = false;
    }

    if (!isa<RegionNode, StartNode>(node.getInputCF())) {
      std::cout << "[IfNode] Expected Region as predicessor. Got: "
                << getOpcName(node.getInputCF()->nodeTy()) << std::endl;
      isValid = false;
    }

    if (node.usersCount() != 2) {
      std::cout << "[IfNode] Invalid users count(!= 2): " << node.opCount()
                << std::endl;
      isValid = false;
    }

    bool IfTrue = false;
    bool IfFalse = false;
    for (auto &&s : node.users()) {
      IfTrue |= isa<IfTrueNode>(s);
      IfFalse |= isa<IfFalseNode>(s);
    }
    if (!IfTrue) {
      std::cout << "[IfNode] Has not IfTrue in successors!" << std::endl;
      isValid = false;
    }
    if (!IfFalse) {
      std::cout << "[IfNode] Has not IfFalse in successors!" << std::endl;
      isValid = false;
    }
  }

  void visitPhiNode(PhiNode &node) {
    // verify input CF
    auto inputCF = node.getInput();
    expected(isa<RegionNode>(inputCF),
             "[Phi] Expected region as input CF node");

    expected(node.numVals() == inputCF->predicessors().size(),
             "[Phi] Number region predecessors differs number input values");

    // verify types
    for (int i = 0; i < node.numVals(); ++i) {
      expected(node.getVal(i) != nullptr &&
                   node.getVal(i)->valueTy() == node.valueTy(),
               "[Phi] Type of input value differs from resut type of Phi Node");
    }
  }
};

} // namespace

bool Function::verify() {
  GraphChecker checker;
  checker.visit(*this);
  return checker.isValid;
}

Function::DFSResultTy Function::dfs() const {
  std::vector<RegionNodeBase *> order;
  // index in 'order'
  std::vector<size_t> parents;
  std::unordered_map<RegionNodeBase *, size_t> visited;

  // first - dfs number of parent
  // second - todo node
  std::vector<std::pair<size_t, RegionNodeBase *>> workList = {
      std::make_pair(0, m_start.get())};

  while (!workList.empty()) {
    auto &&curWork = workList.back();
    auto *V = curWork.second;
    workList.pop_back();
    if (visited.count(V)) {
      continue;
    }

    const auto idx = order.size();
    order.push_back(V);
    parents.push_back(curWork.first);
    visited.emplace(V, idx);

    // insert successors
    for (auto &&s : V->successors()) {
      workList.emplace_back(idx, s);
    }
  }

  return {order, parents, visited};
}

// The compress function recursively traverses the ancestors of a vertex to
// flatten the tree structure:
static void _compress(size_t vDFS, std::vector<size_t> &ancestors,
                      std::vector<size_t> &labels,
                      const std::vector<size_t> &semi) {
  if (ancestors[ancestors[vDFS]] != 0) {
    // FIXME: recursion
    _compress(ancestors[vDFS], ancestors, labels, semi);
    if (semi[labels[ancestors[vDFS]]] < semi[labels[vDFS]]) {
      labels[vDFS] = labels[ancestors[vDFS]];
    }
    ancestors[vDFS] = ancestors[ancestors[vDFS]];
  }
}

// The eval function, which incorporates path compression, is vital to the
// efficiency of the algorithm:
static size_t _eval(size_t vDFS, std::vector<size_t> &ancestors,
                    std::vector<size_t> &labels,
                    const std::vector<size_t> &semi) {
  if (ancestors[vDFS] == 0) {
    return vDFS;
  }

  // perform path compression
  _compress(vDFS, ancestors, labels, semi);

  // if (semi[labels[vDFS]] > semi[labels[ancestors[vDFS]]]) {
  //   return labels[ancestors[vDFS]];
  // }
  return labels[vDFS];
}

/*
### Computing Semi-Dominators:

1. **Initialization**:
    - Create an array/list `semi` of size equal to the number of nodes in the
CFG. Initialize each entry of `semi` to the DFS number of the corresponding
node. This essentially means that initially, every node is its own
semi-dominator.
    - Let `pred` be a list such that `pred[v]` contains all predecessors of node
`v` in the CFG.
    - Let `vertex` be a list such that `vertex[i]` is the node with DFS number
`i`.

2. **Iterate in Reverse DFS Order**:
    - For each node `v` in reverse DFS order (excluding the start node):
        1. For each predecessor `u` of `v` (i.e., for each `u` in `pred[v]`):
            - If `u` has a smaller DFS number than `v` (which means it was
              visited before `v` during DFS), then the semi-dominator of `v` is
              the minimum of `semi[v]` and `semi[u]`.
            - Else, let `u'` be the node with the smallest DFS number reached by
              a sequence of tree edges starting at `u` and ending at a node
              whose DFS number is less than `v`. The semi-dominator of `v` is
              the minimum of `semi[v]` and the DFS number of `u'`.
        2. After processing all predecessors, set `semi[v]` to the node with DFS
              number `semi[v]` (i.e., `semi[v] = vertex[semi[v]]`).
*/
Function::SemiDomResultTy
Function::semiDominators(const DFSResultTy &dfsResult) const {
  auto &&[vertex, dfsParents, dfsNumber] = dfsResult;
  const auto N = vertex.size();
  // initialization
  std::vector<size_t> semi(N);
  std::iota(semi.begin(), semi.end(), 0);

  auto ancestors = dfsParents;
  // std::vector<size_t> ancestors(N, 0);
  std::vector<size_t> labels(N);
  std::iota(labels.begin(), labels.end(), 0);

  // Iterate in Reverse DFS Order:
  for (auto vDFS = N - 1; vDFS > 0; --vDFS) {
    auto *V = vertex[vDFS];
    // TODO: verify that all operand of CFNode is CFNode
    for (auto &&U : V->predicessors()) {
      auto uDFS = dfsNumber.at(U);
      if (uDFS < vDFS) {
        semi[vDFS] = std::min(semi[vDFS], semi[uDFS]);
      } else {
        // search `u'`
        // uDFS = _eval(vDFS, ancestors, labels, semi);
        // NOTE: we can do it faster with _eval
        while (uDFS >= vDFS) {
          uDFS = dfsParents[uDFS];
        }
        semi[vDFS] = std::min(semi[vDFS], semi[uDFS]);
      }
    }
  }

  return semi;
}

/*
Lengauer-Tarjan Algorithm

1. **Initialization**:
    - Create an array (or equivalent data structure) called `idom[]` that will
      hold the immediate dominator of each node. Initially, all entries are set
      to `null` (or an equivalent non-value).
    - Set up an empty list or array `bucket[]` such that `bucket[v]` will
      temporarily hold nodes for which `v` is a candidate immediate dominator.

2. **Populating Buckets**:
    - Process each node `v` in reverse DFS order (except the entry node). For
      each node:
        - Add `v` to `bucket[semi[v]]`. This means that `v` considers `semi[v]`
          as a candidate immediate dominator.

3. **Computing Immediate Dominators**:
    - For each node `w` in reverse DFS order (except the entry node):
        - For each `v` in `bucket[w]`:
            - Find `u`, the vertex in the DFS subtree rooted at `v` (including
              `v`) with the smallest-numbered semi-dominator. This can be found
              using a union-find data structure (also known as disjoint-set)
              that's been augmented with path compression.
            - If `semi[u] < semi[v]`, then `idom[v] = u`. Otherwise, `idom[v] =
              w`.
        - Clear `bucket[w]` to free up space.
        - If `w` is not the root of the DFS tree, add `w` to `bucket[idom[w]]`.

4. **Final Adjustments**:
    - For each node `v` (excluding the entry node and processed in non-reverse
      DFS order), if `idom[v]` is not equal to `semi[v]`, set `idom[v] =
      idom[idom[v]]`.

At the end of this process, the `idom[]` array contains the immediate dominators
for each node in the CFG.
*/
Function::IDomResultTy
Function::iDominators(const DFSResultTy &dfsResult,
                      const SemiDomResultTy &semi) const {
  auto &&[vertex, dfsParents, dfsNumber] = dfsResult;
  const auto N = semi.size();
  std::vector<size_t> idom(N, 0);
  std::unordered_map<size_t, std::vector<size_t>> bucket;

  // Populating Buckets
  for (auto vDFS = N - 1; vDFS > 0; --vDFS) {
    bucket[semi[vDFS]].push_back(vDFS);
  }

  // FIXME: in original paper semidominators and idmo are computed in one loop.
  std::vector<size_t> ancestors(N, 0);
  std::vector<size_t> labels(N);
  std::iota(labels.begin(), labels.end(), 0);
  // Computing Immediate Dominators
  for (auto wDFS = N - 1; wDFS > 0; --wDFS) {
    for (auto &&vDFS : bucket[wDFS]) {
      auto uDFS = _eval(vDFS, ancestors, labels, semi);

      if (semi[uDFS] < semi[vDFS]) {
        idom[vDFS] = uDFS;
      } else {
        idom[vDFS] = wDFS;
      }
    }
  }

  // Final Adjustments
  for (size_t vDFS = 1; vDFS < N; ++vDFS) {
    if (idom[vDFS] != semi[vDFS]) {
      idom[vDFS] = idom[idom[vDFS]];
    }
  }

  return idom;
}

} // namespace son