#pragma once

#include "Node.hpp"

#include <memory>
#include <string>
#include <vector>

namespace son {

struct FunctionType final {
  ValueType m_retType = ValueType::Void;
  std::vector<ValueType> m_argsTypes;

  FunctionType(ValueType ret, const std::vector<ValueType> &args)
      : m_retType(ret), m_argsTypes(args) {}
};

class Function final {
  std::string m_name;
  FunctionType m_fnType;
  std::unique_ptr<StartNode> m_start;
  std::unique_ptr<EndNode> m_end;
  std::vector<FunctionArgNode *> m_args;

  std::vector<std::unique_ptr<Node>> m_graph;

public:
  Function(std::string name, FunctionType fnType)
      : m_name(std::move(name)), m_fnType(std::move(fnType)),
        m_start(std::make_unique<StartNode>()),
        m_end(std::make_unique<EndNode>()) {
    createArgs();
  }

private:
  void createArgs() {
    assert(m_args.empty());
    for (auto &&ty : m_fnType.m_argsTypes) {
      assert(ty != ValueType::Void);
      m_args.emplace_back(create<FunctionArgNode>(ty));
    }
  }

public:
  template <typename T, typename... Args> T *create(Args &&...args) {
    auto &&uptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto ptr = uptr.get();
    m_graph.emplace_back(std::move(uptr));
    return ptr;
  }

  StartNode *getStart() { return m_start.get(); }
  EndNode *getEnd() { return m_end.get(); }

  auto getArg(size_t id) { return m_args.at(id); }
  auto getNumArgs() const { return m_args.size(); }

  auto begin() { return m_graph.begin(); }
  auto end() { return m_graph.end(); }

  //=------------------------------------------------------------------
  // Verification
  //=------------------------------------------------------------------
  bool verify();

  //=------------------------------------------------------------------
  // ALGORITHMS
  //=------------------------------------------------------------------

  // 0 - \vec[i] has node with dfs number \i
  // 1 - parents[i] has dfs number of parent of node with dfs number i.
  // 2 - map Node* to its DFS number
  // NOTE: parents[0] always equal 0
  using DFSResultTy =
      std::tuple<std::vector<RegionNodeBase *>, std::vector<size_t>,
                 std::unordered_map<RegionNodeBase *, size_t>>;
  // vec[i] - dfs number of semi-dominatro for node with DFS number `i`
  using SemiDomResultTy = std::vector<size_t>;
  // vec[i] - idom DFS number for node with DFS number `i`
  using IDomResultTy = std::vector<size_t>;

  DFSResultTy dfs() const;
  SemiDomResultTy semiDominators(const DFSResultTy &dfsResult) const;
  IDomResultTy iDominators(const DFSResultTy &dfsResult,
                           const SemiDomResultTy &semi) const;
};

} // namespace son