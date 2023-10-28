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

  // Define visitors for functions and basic blocks...
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

  // visit - Finally, code to visit an instruction...
  //
  RetTy visit(Node &I) {
    static_assert(std::is_base_of<InstVisitor, SubClass>::value,
                  "Must pass the derived type to this template!");

    switch (I.nodeTy()) {
    default:
      assert(0 && "Unknown instruction type encountered!");
      // Build the switch statement using the Instruction.def file...
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

  //===--------------------------------------------------------------------===//
  // Visitation functions... these functions provide default fallbacks in case
  // the user does not specify what to do for a particular instruction type.
  // The default behavior is to generalize the instruction type to its subtype
  // and try visiting the subtype.  All of this should be inlined perfectly,
  // because there are no virtual functions to get in the way.
  //

  // When visiting a module, function or basic block directly, these methods get
  // called to indicate when transitioning into a new unit.
  //
  // TODO: void visitModule(Module &M) {}
  void visitFunction(Function &F) {}
  void visitBasicBlock(Node &BB) {}

  // Define instruction specific visitor functions that can be overridden to
  // handle SPECIFIC instructions.  These functions automatically define
  // visitMul to proxy to visitBinaryOperator for instance in case the user does
  // not need this generality.
  //
  // These functions can also implement fan-out, when a single opcode and
  // instruction have multiple more specific Instruction subclasses. The Call
  // instruction currently supports this. We implement that by redirecting that
  // instruction to a special delegation helper.
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

  // Specific Instruction type classes... note that all of the casts are
  // necessary because we use the instruction classes as opaque types...
  //
  // RetTy visitICmpInst(CFNode &I) { DELEGATE(CmpInst); }

  // Next level propagators: If the user does not overload a specific
  // instruction type, they can overload one of these to get the whole class
  // of instructions...
  //
  RetTy visitCastOperation(CastOperationNode &I) {
    DELEGATE(CastOperationNode);
  }
  RetTy visitBinaryOperator(BinOpNode &I) { DELEGATE(BinOpNode); }

  // If the user wants a 'default' case, they can choose to override this
  // function.  If this function is not overloaded in the user's subclass, then
  // this instruction just gets ignored.
  //
  // Note that you MUST override this function if your return type is not void.
  //
  void visitInstruction(Node &I) {} // Ignore unhandled instructions

};

#undef DELEGATE
} // namespace son