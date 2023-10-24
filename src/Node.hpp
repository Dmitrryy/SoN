#pragma once

#include "Arena.hpp"
#include "Opcodes.hpp"

#include <algorithm>
#include <cassert>
#include <list>
#include <vector>

namespace son {

class User;
class Function;

//=------------------------------------------------------------------
// data nodes
//=------------------------------------------------------------------

class Value {
  ValueType m_valType = ValueType::Void;
  NodeType m_nodeType = NodeType::Unknown;
  std::vector<User *> m_useList;

  friend User;

protected:
  // Value() = default;
  Value(NodeType nodeType, ValueType valType)
      : m_nodeType(nodeType), m_valType(valType) {}

public:
  virtual ~Value() = default;

  const auto getNodeType() const noexcept { return m_nodeType; }
  const auto getValueType() const noexcept { return m_valType; }
  const auto &getUses() const noexcept { return m_useList; }
  const auto getNumUses() const noexcept { return m_useList.size(); }

protected:
  void addUse(User &U) { m_useList.push_back(&U); }
  void eraseUse(User &U) {
    auto &&it = std::find(m_useList.begin(), m_useList.end(), &U);
    m_useList.erase(it);
  }
}; // class Value

class User : public Value {
  std::vector<Value *> m_operands;

public:
  User(NodeType nodeType, ValueType valType, size_t numOps)
      : Value(nodeType, valType), m_operands(numOps, nullptr) {}

  auto getNumOperands() const { return m_operands.size(); }
  auto getOperand(size_t id) const { return m_operands.at(id); }
  auto setOperand(size_t id, Value *val) {
    if (auto *OldOp = m_operands.at(id)) {
      OldOp->eraseUse(*this);
    }
    val->addUse(*this);
    return m_operands.at(id) = val;
  }

protected:
  void addOperand(Value *val) {
    val->addUse(*this);
    m_operands.push_back(val);
  }
}; // class User

//=------------------------------------------------------------------
// constant
// class UnknownNode : public Value {};

template <typename T, ValueType ValTy> class ConstantNode : public Value {
  T m_val = {};
  friend Function;

public:
  ConstantNode(T val) : Value(NodeType::Constant, ValTy), m_val(val) {}

  void setConstant(T val) { m_val = val; }
  T getConstant() const { return m_val; }
};

using CInt8Node = ConstantNode<int8_t, ValueType::Int32>;
using CInt32Node = ConstantNode<int32_t, ValueType::Int32>;
using CInt64Node = ConstantNode<int64_t, ValueType::Int64>;

//=------------------------------------------------------------------
// function
class FunctionArgNode : public Value {
  friend Function;

public:
  FunctionArgNode(ValueType type) : Value(NodeType::FunctionArg, type) {}
}; // FunctionArgNode

//=------------------------------------------------------------------
// un operations
class TrunkNode : public User {
  friend Function;

public:
  TrunkNode(Value *operand, ValueType resType)
      : User(NodeType::Trunk, resType, 1) {
    assert(operand->getValueType() != ValueType::Void);
    // TODO: check that new size lower: assert(???);
    setOperand(0, operand);
  }
}; // TrunkNode

class ZextNode : public User {
  friend Function;

public:
  ZextNode(Value *operand, ValueType resType)
      : User(NodeType::Zext, resType, 1) {
    assert(operand->getValueType() != ValueType::Void);
    assert(resType != ValueType::Void);
    // TODO: check that new size larger: assert(???);
    setOperand(0, operand);
  }
}; // ZextNode

class SextNode : public User {
  friend Function;

public:
  SextNode(Value *operand, ValueType resType)
      : User(NodeType::Sext, resType, 1) {
    assert(operand->getValueType() != ValueType::Void);
    assert(resType != ValueType::Void);
    // TODO: check that new size larger: assert(???);
    setOperand(0, operand);
  }
}; // SextNode

class NotNode : public User {
  friend Function;

public:
  NotNode(Value *operand) : User(NodeType::Not, operand->getValueType(), 1) {
    assert(operand->getValueType() != ValueType::Void);
    setOperand(0, operand);
  }
}; // NotNode

class NegNode : public User {
  friend Function;

public:
  NegNode(Value *operand) : User(NodeType::Neg, operand->getValueType(), 1) {
    assert(operand->getValueType() != ValueType::Void);
    setOperand(0, operand);
  }
}; // NegNode

//=------------------------------------------------------------------
// bin operations
class BinOpNode : public User {
protected:
  BinOpNode(Value *lhs, Value *rhs, NodeType op)
      : User(op, lhs->getValueType(), 2) {
    assert(lhs->getValueType() == rhs->getValueType());
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
    opc_name##Node(Value *lhs, Value *rhs)                                     \
        : BinOpNode(lhs, rhs, NodeType::opc_name) {}                           \
  };

#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_BIN_DEFINE

//=------------------------------------------------------------------
// control flow nodes
//=------------------------------------------------------------------
class CFNode : public User {
  friend Function;

public:
  CFNode(NodeType nodeType, ValueType valType, size_t numOps)
      : User(nodeType, valType, numOps) {}
};

class StartNode : public CFNode {
  friend Function;

public:
  StartNode() : CFNode(NodeType::Start, ValueType::Void, 0) {}
}; // class StartNode

class EndNode : public CFNode {
  friend Function;

public:
  EndNode() : CFNode(NodeType::End, ValueType::Void, 0) {}

  // operand[0...] - input control flows
  void addCFInput(CFNode *input) { addOperand(input); }

}; // class EndNode

class RangeNode : public CFNode {
  friend Function;

public:
  RangeNode() : CFNode(NodeType::End, ValueType::Void, 0) {}

  void addCFInput(CFNode *input) { addOperand(input); }
}; // class RangeNode

class PhiNode : public CFNode {
  friend Function;

public:
  PhiNode(CFNode *inputCF, size_t numInputs, ValueType type)
      : CFNode(NodeType::Phi, type, numInputs + 1) {
    setOperand(0, inputCF);
  }

  void setVal(size_t id, Value *val) { setOperand(id + 1, val); }

  void setInput(size_t id, Value *val) { setVal(id, val); }
}; // class PhiNode

class IfNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  // operand[1]{Value} - condition
  IfNode(CFNode *input, Value *cond)
      : CFNode(NodeType::If, ValueType::Void, 2) {
    setInputCF(input);
    setCondition(cond);
  }

  void setInputCF(CFNode *input) { setOperand(0, input); }
  void setCondition(Value *cond) { setOperand(1, cond); }
}; // class IfNode

class IfFalseNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  IfFalseNode(CFNode *input) : CFNode(NodeType::IfFalse, ValueType::Void, 1) {
    setInputCF(input);
  }
  void setInputCF(CFNode *input) { setOperand(0, input); }
}; // class IfFalseNode

class IfTrueNode : public CFNode {
  friend Function;

public:
  // operand[0]{CFNode} - input control flow
  IfTrueNode(CFNode *input) : CFNode(NodeType::IfTrue, ValueType::Void, 1) {
    setInputCF(input);
  }
  void setInputCF(CFNode *input) { setOperand(0, input); }
}; // class IfFalseNode

class RetNode : public CFNode {
  friend Function;

public:
  RetNode() : CFNode(NodeType::Ret, ValueType::Void, 0) {}
  RetNode(Value *val) : CFNode(NodeType::Ret, val->getValueType(), 1) {
    setRetValue(val);
  }

  void setRetValue(Value *val) { setOperand(0, val); }
  auto getRetValue() { return getOperand(0); }
};

} // namespace son