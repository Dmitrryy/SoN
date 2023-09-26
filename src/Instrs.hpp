#pragma once

#include "Node.hpp"

#include <vector>

namespace son {

class BinOp : public Node, public ArgsBuff<2> {
  Type m_Type = Type::Void;
  BinOpIntrs m_BinOpcode = BinOpIntrs::UNKNOWN;
  Node *m_ctrl = nullptr;

public:
  BinOp(BinOpIntrs op, Type ty, Node *arg1, Node *arg2, Node *ctrl)
      : Node(NodeType::BinOp), m_Type(ty), m_BinOpcode(op), m_ctrl(ctrl) {
    m_inputs = {arg1, arg2};
  }
}; // class BinOp

// class UnOp : public Node, public ArgsBuff<1> {
//     UnOpInstrs op;
// public:
//   UnOp(UnOpInstrs op, Type ty, Node *arg1, Node *ctrl) : Node(op, ty) {
//     m_inputs = {arg1, ctrl};
//   }
// }; // class BinOp

class ConstInt32 final : public Node {
  int32_t m_cnst = {};
  Node *m_ctrl = nullptr;

public:
  ConstInt32(int32_t val)
      : Node(NodeType::Const), m_cnst(val) {}
};

class PhiNode final : public Node {
  Node *m_ctrl = nullptr;
  std::vector<Node *> m_inputs;

public:
  PhiNode(Node *ctrl, std::vector<Node *> inputs)
      : Node(NodeType::PHI), m_ctrl(ctrl), m_inputs(inputs) {}
};

// control-flow


class ControlNode : public Node {
  ControlNodeOp m_CtrlNodeOp = ControlNodeOp::UNKNOWN;

public:
  ControlNode(ControlNodeOp op) : Node(NodeType::Control), m_CtrlNodeOp(op) {}
};

class Region final : public Node {
  std::vector<ControlNode *> m_inputs;

public:
  Region(std::vector<ControlNode *> inputs = {})
      : Node(NodeType::Region), m_inputs(std::move(inputs)) {}
};

class JmpNode final : public ControlNode {
  Region *m_region = nullptr;

public:
  JmpNode(Region *region) : ControlNode(ControlNodeOp::Jmp), m_region(region) {}
};

class IfNode final : public ControlNode {
  Region *m_region = nullptr;
  Node *m_cond = nullptr;

public:
  IfNode(Region *region, Node *cond)
      : ControlNode(ControlNodeOp::If), m_region(region), m_cond(cond) {}
};

class ReturnNode final : public ControlNode {
  Region *m_region = nullptr;
  Node *m_value = nullptr;

public:
  ReturnNode(Region *region, Node *value)
      : ControlNode(ControlNodeOp::Return), m_region(region), m_value(value) {}
};

} // namespace son