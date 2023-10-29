#pragma once

#include "Function.hpp"

namespace son {
#define DELEGATE(CLASS_TO_VISIT)                                               \
  return static_cast<SubClass *>(this)->visit##CLASS_TO_VISIT(                 \
      static_cast<CLASS_TO_VISIT &>(I))

template <typename SubClass, typename RetTy = void> class InstVisitor {
  //===--------------------------------------------------------------------===//
  // Interface code - This is the public interface of the InstVisitor that you
  // use to visit instructions...
  //

public:
  // Generic visit method - Allow visitation to all instructions in a range
  template <class Iterator> void visit(Iterator Start, Iterator End) {
    while (Start != End) {
      static_cast<SubClass *>(this)->visit(*Start->get());
      ++Start;
    }
  }

  // Define visitors for module and functions...
  //
  // TODO: module
  // void visit(Module &M) {
  //   static_cast<SubClass *>(this)->visitModule(M);
  //   visit(M.begin(), M.end());
  // }
  void visit(Function &F) {
    static_cast<SubClass *>(this)->visitFunction(F);
    visit(F.begin(), F.end());
  }

  // Forwarding functions so that the user can visit with pointers AND refs.
  // TODO: void visit(Module *M) { visit(*M); }
  void visit(Function *F) { visit(*F); }
  RetTy visit(Node *I) { return visit(*I); }

  // visit - Finally, code to visit an node...
  //
  RetTy visit(Node &I) {
    static_assert(std::is_base_of<InstVisitor, SubClass>::value,
                  "Must pass the derived type to this template!");

    switch (I.nodeTy()) {
    default:
      assert(0 && "Unknown instruction type encountered!");
      // Build the switch statement using the Opcodes.def file...
#define NODE_OPCODE_ALL_DEFINE
#define NODE_OPCODE_DEFINE(opc_name)                                           \
  case NodeType::opc_name:                                                     \
    return static_cast<SubClass *>(this)->visit##opc_name(                     \
        static_cast<opc_name##Node &>(I));
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_ALL_DEFINE
    }
  }

  // When visiting a module or function directly, these methods get
  // called to indicate when transitioning into a new unit.
  //
  // TODO: void visitModule(Module &M) {}
  void visitFunction(Function &F) {}

  // visitors for each opcode
#define NODE_OPCODE_ALL_DEFINE
#define NODE_OPCODE_DEFINE(OPCODE)                                             \
  RetTy visit##OPCODE(OPCODE##Node &I) {                                       \
    DELEGATE(OPCODE##Node);                                                    \
  }
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_ALL_DEFINE

#define NODE_OPCODE_ALL_DEFINE
#define NODE_OPCODE_DEFINE(OPCODE)                                             \
  RetTy visit##OPCODE##Node(OPCODE##Node &I) { }
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_ALL_DEFINE

  // Specific Instruction type classes... 
  RetTy visitCastOperation(CastOperationNode &I) {
    DELEGATE(CastOperationNode);
  }
  RetTy visitBinaryOperator(BinOpNode &I) { DELEGATE(BinOpNode); }
};

#undef DELEGATE
} // namespace son