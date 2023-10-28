#include "Function.hpp"
#include "InstVisitor.hpp"

#include <iostream>
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
      expected(isa<JmpNode, IfTrueNode, IfFalseNode, StartNode>(p),
               "Unexpected Region predicessor: " + getOpcName(p->nodeTy()));
    }

    for (auto &&s : node.users()) {
      expected(
          isa<JmpNode, IfNode, IfTrueNode, IfFalseNode, PhiNode, RetNode>(s),
          "Unexpected Region successor: " + getOpcName(s->nodeTy()));
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
      expected(node.getVal(i) != nullptr && node.getVal(i)->valueTy() == node.valueTy(),
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

std::vector<std::pair<Node *, size_t>> Function::dfs() const {
  std::vector<std::pair<Node *, size_t>> order;
  std::unordered_map<Node *, size_t> visited;
  std::vector<Node *> workList = {m_start.get()};

  while (!workList.empty()) {
    auto *V = workList.back();
    workList.pop_back();
    if (visited.count(V)) {
      continue;
    }

    const auto Id = order.size();
    order.push_back(std::make_pair(V, Id));
    visited.emplace(V, Id);

    // insert successors
    for (auto &&s : V->users()) {
      // FIXME: if Control Flow Node
      if (isa<StartNode, RegionNode, JmpNode, RetNode, IfNode, IfTrueNode,
              IfFalseNode>(s)) {
        workList.push_back(s);
      }
    }
  }

  return order;
}

} // namespace son