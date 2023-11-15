#pragma once

#include "Arena.hpp"
#include "Opcodes.hpp"

#include <algorithm>
#include <cassert>
#include <list>
#include <unordered_set>
#include <vector>

namespace son {

class User;
class Function;
class CFNode;
class RegionNode;
class PhiNode;

//=------------------------------------------------------------------
// data nodes
//=------------------------------------------------------------------

class Node {
  // Inputs
  std::vector<Node *> m_operands;
  // Outputs
  // We should rapidly remove and add users, but order is not important.
  std::unordered_multiset<Node *> m_users;

  ValueType m_valType = ValueType::Void;
  NodeType m_nodeType = NodeType::Unknown;

protected:
  Node(NodeType nTy) : m_nodeType(nTy) {}
  Node(NodeType nTy, ValueType vTy) : m_nodeType(nTy), m_valType(vTy) {}
  Node(NodeType nTy, ValueType vTy, size_t nOperands)
      : m_operands(nOperands, nullptr), m_valType(vTy), m_nodeType(nTy) {}

public:
  virtual ~Node() = default;

public:
  const auto nodeTy() const noexcept { return m_nodeType; }
  const auto valueTy() const noexcept { return m_valType; }

  auto opCount() const noexcept { return m_operands.size(); }
  const auto &operands() const noexcept { return m_operands; }
  auto operand(size_t idx) const { return m_operands[idx]; }
  auto usersCount() const noexcept { return m_users.size(); }
  const auto &users() const noexcept { return m_users; }

  //=------------------------------------------------------------------
  // Debug functions
  //=------------------------------------------------------------------
  virtual void
  dump(std::ostream &stream,
       const std::unordered_map<const Node *, std::string> &names) const {
    if (valueTy() != ValueType::Void) {
      stream << names.at(this) << " = " << getTyName(valueTy()) << ' ';
    }
    stream << getOpcName(nodeTy());
    for (int i = 0; i < opCount(); ++i) {
      auto opr = operand(i);
      stream << ((i == 0) ? " " : ", ");
      stream << getTyName(opr->valueTy()) << ' ' << names.at(opr);
    }
  }

protected:
  auto addOperand(Node *operand) {
    auto idx = m_operands.size();
    m_operands.push_back(operand);

    // register the usage of operand
    if (operand != nullptr) {
      operand->m_users.emplace(this);
    }
    return idx;
  }
  void setOperand(size_t idx, Node *operand) {
    // remove usage of previous operand
    auto &&opPlace = m_operands.at(idx);
    if (opPlace != nullptr) {
      opPlace->m_users.erase(m_users.find(this));
    }
    // sometimes we want remain operand empty for a while
    if (operand != nullptr) {
      operand->m_users.emplace(this);
    }
    opPlace = operand;
  }

  // NOTE: expensive operation.
  Node *removeOperand(size_t idx) {
    auto *operand = m_operands.at(idx);
    if (operand != nullptr) {
      operand->m_users.erase(m_users.find(this));
    }

    m_operands.erase(m_operands.begin() + idx);
    return operand;
  }

  void setValueType(ValueType vTy) { m_valType = vTy; }
}; // class Node

//=------------------------------------------------------------------
// constant
class ConstantNode : public Node {
  uint64_t m_val = {};
  friend Function;

public:
  ConstantNode(ValueType vTy, uint64_t val)
      : Node(NodeType::Constant, vTy), m_val(val) {}

  void setConstant(uint64_t val) { m_val = val; }
  uint64_t getConstant() const { return m_val; }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Constant;
  }

  void dump(std::ostream &stream,
            const std::unordered_map<const Node *, std::string> &names)
      const override {
    stream << names.at(this) << " = " << getTyName(valueTy()) << ' '
           << std::to_string(m_val);
  }
};

//=------------------------------------------------------------------
// function
class FunctionArgNode : public Node {
  friend Function;

public:
  FunctionArgNode(ValueType type) : Node(NodeType::FunctionArg, type) {}

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::FunctionArg;
  }
}; // FunctionArgNode

//=------------------------------------------------------------------
// cast operations
class CastOperationNode : public Node {
  ValueType m_dstTy = ValueType::Void;
  ValueType m_srcTy = ValueType::Void;

protected:
  CastOperationNode(NodeType nTy, Node *val, ValueType srcTy, ValueType dstTy)
      : Node(nTy, dstTy, 1), m_dstTy(dstTy), m_srcTy(srcTy) {}

public:
  auto value() const noexcept { return operand(0); }
  void setValue(Node *val) { setOperand(0, val); }
  auto dstTy() const noexcept { return m_dstTy; }
  auto srcTy() const noexcept { return m_srcTy; }
};

class TrunkNode : public CastOperationNode {
  friend Function;

public:
  TrunkNode(Node *operand, ValueType resTy)
      : CastOperationNode(NodeType::Trunk, operand, operand->valueTy(), resTy) {
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Trunk;
  }
}; // TrunkNode

class ZextNode : public CastOperationNode {
  friend Function;

public:
  ZextNode(Node *operand, ValueType resTy)
      : CastOperationNode(NodeType::Zext, operand, operand->valueTy(), resTy) {}

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Zext;
  }
}; // ZextNode

class SextNode : public CastOperationNode {
  friend Function;

public:
  SextNode(Node *operand, ValueType resTy)
      : CastOperationNode(NodeType::Sext, operand, operand->valueTy(), resTy) {}

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Sext;
  }
}; // SextNode

class BitCastNode : public CastOperationNode {
  friend Function;

public:
  BitCastNode(Node *operand, ValueType resTy)
      : CastOperationNode(NodeType::Sext, operand, operand->valueTy(), resTy) {}

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Sext;
  }
}; // SextNode

//=------------------------------------------------------------------
// un operations
class NotNode : public Node {
  friend Function;

public:
  NotNode(Node *operand) : Node(NodeType::Not, operand->valueTy(), 1) {
    assert(operand->valueTy() != ValueType::Void);
    setOperand(0, operand);
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Not;
  }
}; // NotNode

class NegNode : public Node {
  friend Function;

public:
  NegNode(Node *operand) : Node(NodeType::Neg, operand->valueTy(), 1) {
    assert(operand->valueTy() != ValueType::Void);
    setOperand(0, operand);
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Neg;
  }
}; // NegNode

//=------------------------------------------------------------------
// bin operations
class BinOpNode : public Node {
protected:
  BinOpNode(Node *lhs, Node *rhs, NodeType op) : Node(op, lhs->valueTy(), 2) {
    assert(lhs->valueTy() == rhs->valueTy());
    setOperand(0, lhs);
    setOperand(1, rhs);
  }
}; // class BinOpNode

#define NODE_OPCODE_BIN_DEFINE
#define NODE_OPCODE_DEFINE(opc_name)                                           \
  class opc_name##Node : public BinOpNode {                                    \
    friend Function;                                                           \
                                                                               \
  public:                                                                      \
    opc_name##Node(Node *lhs, Node *rhs)                                       \
        : BinOpNode(lhs, rhs, NodeType::opc_name) {}                           \
    static bool classof(const Node *node) noexcept {                           \
      return node->nodeTy() == NodeType::opc_name;                             \
    }                                                                          \
  };

#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_BIN_DEFINE

//=------------------------------------------------------------------
// bin operations
class CmpNode : public Node {
protected:
  CmpNode(Node *lhs, Node *rhs, NodeType op) : Node(op, ValueType::Int1, 2) {
    assert(lhs->valueTy() == rhs->valueTy());
    setOperand(0, lhs);
    setOperand(1, rhs);
  }
}; // class BinOpNode

#define NODE_OPCODE_CMP_DEFINE
#define NODE_OPCODE_DEFINE(opc_name)                                           \
  class opc_name##Node : public CmpNode {                                      \
    friend Function;                                                           \
                                                                               \
  public:                                                                      \
    opc_name##Node(Node *lhs, Node *rhs)                                       \
        : CmpNode(lhs, rhs, NodeType::opc_name) {}                             \
    static bool classof(const Node *node) noexcept {                           \
      return node->nodeTy() == NodeType::opc_name;                             \
    }                                                                          \
  };
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_CMP_DEFINE

//=------------------------------------------------------------------
// control flow nodes
//=------------------------------------------------------------------
class CFNode : public Node {
  friend Function;

public:
  CFNode(NodeType nodeType, ValueType valType, size_t numOps)
      : Node(nodeType, valType, numOps) {}
};

class RegionNodeBase : public CFNode {
public:
  RegionNodeBase(NodeType nTy) : CFNode(nTy, ValueType::Void, 0) {}

  std::vector<PhiNode *> phis() const;
  CFNode *terminator() const;
  std::vector<RegionNodeBase *> predicessors() const;
  std::vector<RegionNodeBase *> successors() const;
};

class RegionNode : public RegionNodeBase {
  friend Function;

public:
  RegionNode() : RegionNodeBase(NodeType::Region) {}

  template <typename... Args> void addCFInput(CFNode *input, Args... args) {
    addOperand(input);
    addCFInput(args...);
  }
  template <> void addCFInput(CFNode *input) { addOperand(input); }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Region;
  }
}; // class RegionNode

class StartNode : public RegionNodeBase {
  friend Function;

public:
  StartNode() : RegionNodeBase(NodeType::Start) {}
  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Start;
  }
}; // class StartNode

class EndNode : public RegionNodeBase {
  friend Function;

public:
  EndNode() : RegionNodeBase(NodeType::End) {}

  template <typename... Args> void addCFInput(CFNode *input, Args... args) {
    addOperand(input);
    addCFInput(args...);
  }
  template <> void addCFInput(CFNode *input) { addOperand(input); }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::End;
  }
}; // class EndNode

//=------------------------------------------------------------------
// control: If, Jmp, Ret
class IfTrueNode;
class IfFalseNode;

class IfNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  // operand[1]{Value} - condition
  IfNode(RegionNodeBase *input, Node *cond)
      : CFNode(NodeType::If, ValueType::Void, 2) {
    setInputCF(input);
    setCondition(cond);
  }

  void setInputCF(RegionNodeBase *input) { setOperand(0, input); }
  auto getInputCF() const { return dynamic_cast<RegionNodeBase *>(operand(0)); }
  void setCondition(Node *cond) { setOperand(1, cond); }
  auto getCondition() const { return operand(1); }

  RegionNodeBase *getTrueExit() const {
    for (auto &&U : users()) {
      if (isa<IfTrueNode>(U)) {
        auto *res = dynamic_cast<RegionNodeBase *>(*U->users().begin());
        assert(res);
        return res;
      }
    }
    return nullptr;
  }
  RegionNodeBase *getFalseExit() const {
    for (auto &&U : users()) {
      if (isa<IfFalseNode>(U)) {
        auto *res = dynamic_cast<RegionNodeBase *>(*U->users().begin());
        assert(res);
        return res;
      }
    }
    return nullptr;
  }

  void dump(std::ostream &stream,
            const std::unordered_map<const Node *, std::string> &names)
      const override {
    stream << getOpcName(nodeTy());

    auto *TN = getTrueExit();
    auto *FN = getFalseExit();
    stream << ' ' << getTyName(getCondition()->valueTy()) << ' '
           << names.at(getCondition()) << ", T:" << names.at(TN)
           << ", F:" << names.at(FN);
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::If;
  }
}; // class IfNode

class IfFalseNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  IfFalseNode(CFNode *input) : CFNode(NodeType::IfFalse, ValueType::Void, 1) {
    setInputCF(input);
  }
  void setInputCF(CFNode *input) { setOperand(0, input); }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::IfFalse;
  }
}; // class IfFalseNode

class IfTrueNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  IfTrueNode(CFNode *input) : CFNode(NodeType::IfTrue, ValueType::Void, 1) {
    setInputCF(input);
  }
  void setInputCF(CFNode *input) { setOperand(0, input); }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::IfTrue;
  }
}; // class IfFalseNode

class JmpNode : public CFNode {
  friend Function;

public:
  JmpNode(CFNode *input) : CFNode(NodeType::Jmp, ValueType::Void, 1) {
    setOperand(0, input);
  }

  void dump(std::ostream &stream,
            const std::unordered_map<const Node *, std::string> &names)
      const override {
    stream << getOpcName(nodeTy());

    stream << ' ' << names.at(*users().begin());
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Jmp;
  }
}; // class JmpNode

class RetNode : public CFNode {
  friend Function;

public:
  RetNode(CFNode *input) : CFNode(NodeType::Ret, ValueType::Void, 1) {
    setInputCF(input);
  }
  RetNode(CFNode *input, Node *val)
      : CFNode(NodeType::Ret, ValueType::Void, 2) {
    setInputCF(input);
    setRetValue(val);
  }

  void setInputCF(CFNode *node) { setOperand(0, node); }
  auto getInputCF() const { return dynamic_cast<RegionNodeBase *>(operand(0)); }
  void setRetValue(Node *val) { setOperand(1, val); }
  auto getRetValue() { return operand(1); }
  auto retTy() const {
    return (opCount() == 1) ? ValueType::Void : operand(1)->valueTy();
  }
  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Ret;
  }
}; // class RetNode

//=------------------------------------------------------------------
// Phi node
class PhiNode : public Node {
  friend Function;

public:
  PhiNode(RegionNode *inputCF, size_t numInputs, ValueType type)
      : Node(NodeType::Phi, type, numInputs + 1) {
    setOperand(0, inputCF);
  }

  auto numVals() const noexcept { return opCount() - 1; }
  void setVal(size_t id, Node *val) { setOperand(id + 1, val); }
  Node *getVal(size_t id) const { return operand(id + 1); }
  void setInput(RegionNode *val) { setOperand(0, val); }
  RegionNode *getInput() const {
    return dynamic_cast<RegionNode *>(operand(0));
  }
  RegionNodeBase *getInputReg(size_t idx) const {
    return getInput()->predicessors().at(idx);
  }

  static bool classof(const Node *node) noexcept {
    return node->nodeTy() == NodeType::Phi;
  }
}; // class PhiNode

} // namespace son