#pragma once

#include "Node.hpp"

#include <memory>
#include <string>
#include <vector>

namespace son {

class DomTree;
class LoopInfo;

struct FunctionType final {
  ValueType retType = ValueType::Void;
  std::vector<ValueType> argsTypes;

  FunctionType(ValueType ret, const std::vector<ValueType> &args)
      : retType(ret), argsTypes(args) {}
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
    for (auto &&ty : m_fnType.argsTypes) {
      assert(ty != ValueType::Void);
      m_args.emplace_back(create<FunctionArgNode>(ty, m_args.size()));
    }
  }

public:
  template <typename T, typename... Args> T *create(Args &&...args) {
    auto &&uptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto ptr = uptr.get();
    m_graph.emplace_back(std::move(uptr));
    return ptr;
  }

  StartNode *getStart() const { return m_start.get(); }
  EndNode *getEnd() const { return m_end.get(); }

  auto getArg(size_t id) { return m_args.at(id); }
  auto getNumArgs() const { return m_args.size(); }

  auto begin() { return m_graph.begin(); }
  auto end() { return m_graph.end(); }

  auto getFnTy() const { return m_fnType; }

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
  // 3 - rpo order
  // NOTE: parents[0] always equal 0
  using DFSResultTy =
      std::tuple<std::vector<RegionNodeBase *>, std::vector<size_t>,
                 std::unordered_map<RegionNodeBase *, size_t>,
                 std::vector<RegionNodeBase *>>;
  // vec[i] - dfs number of semi-dominatro for node with DFS number `i`
  using SemiDomResultTy = std::vector<size_t>;
  // vec[i] - idom DFS number for node with DFS number `i`
  using IDomResultTy = std::vector<size_t>;

  DFSResultTy dfs() const;
  SemiDomResultTy semiDominators(const DFSResultTy &dfsResult) const;
  IDomResultTy iDominators(const SemiDomResultTy &semi) const;

  //=------------------------------------------------------------------
  // Sea of nodes specific functions
  //=------------------------------------------------------------------
  using DataMapperTy =
      std::unordered_map<RegionNodeBase *, std::vector<Node *>>;
  DataMapperTy dataSchedule(const DomTree &DT) const;
  std::vector<RegionNodeBase *>
  linearize(const DomTree &DT, const DFSResultTy &dfs, LoopInfo &LI) const;

  //=------------------------------------------------------------------
  // Debug functions
  //=------------------------------------------------------------------
  using NamesMapTy = std::unordered_map<const Node *, std::string>;
  void dump(std::ostream &stream, const NamesMapTy &names) const;
  void dump(std::ostream &stream) const {
    NamesMapTy names;
    nameNodes(names);
    dump(stream, names);
  }
  void nameNodes(NamesMapTy &names) const;

protected:
  void _dfs(RegionNodeBase *veertex,
            std::unordered_map<RegionNodeBase *, size_t> &visited,
            std::vector<size_t> &parents, std::vector<RegionNodeBase *> &dfs,
            std::vector<RegionNodeBase *> &rpo) const;
};

} // namespace son