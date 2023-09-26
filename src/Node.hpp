#pragma once

#include "Arena.hpp"
#include "Opcodes.hpp"

#include <list>

namespace son {

class Node {
protected:
  static Allocator mg_Allocator;

  NodeType m_NodeTyep = NodeType::UNKNOWN;
//   Opcode m_opcode = Opcode::UNKNOWN;
//   Type m_type = Type::Void;

public:
  Node(NodeType op)
      : m_NodeTyep(op) {}

public:
//   virtual const char *Name() = 0;

public:
  void *operator new(size_t size) { return mg_Allocator.Allocate(size); }
  void operator delete(void *ptr) { mg_Allocator.Deallocate(ptr); }
}; // class Node

template <size_t Size> class ArgsBuff {
protected:
  std::array<Node *, Size> m_inputs;

public:
  Node *getArg(size_t id) { return m_inputs[id]; }
  void setArg(size_t id, Node *node) { m_inputs[id] = node; }
  auto args_begin() const { return m_inputs.begin(); }
  auto args_end() const { return m_inputs.end(); }
}; // class ArgsBuff

} // namespace son