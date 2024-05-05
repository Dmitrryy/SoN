#pragma once

#include "Function.hpp"
#include "Node.hpp"
#include "Opcodes.hpp"

#include <vector>

namespace son {

class Inlining {

public:
  void run(Function &F) {
    std::vector<CallNode *> toInline = _getCandidates(F);

    for (auto *CN : toInline) {
      inlineStaticCall(F, *CN);
    }
  }

private:
  std::vector<CallNode *> _getCandidates(Function &F) const {
    std::vector<CallNode *> toInline;
    for (auto &&n : F) {
      auto *node = n.get();
      if (isa<CallNode>(node)) {
        toInline.emplace_back(dynamic_cast<CallNode *>(node));
      }
    }
    return toInline;
  }

  void inlineStaticCall(Function &F, CallNode &CN) const {
    auto *Callee = CN.getCallee();

    auto &&CalleeCopy = Callee->copy();

    // 1. replace FunctionArgsNode's with values passed to call
    const auto numArgs = CalleeCopy.getNumArgs();
    for (size_t i = 0; i < numArgs; ++i) {
      CalleeCopy.getArg(i)->replaceWith(CN.operand(i));
    }

    // 2. replace CallNode users with PhiNode from returns of inlined function
    const auto numRetValues = CalleeCopy.getEnd()->opCount();
    auto RetRegion = F.create<RegionNode>();
    auto RetValue = F.create<PhiNode>(RetRegion, numRetValues,
                                      CalleeCopy.getFnTy().retType);
    for (size_t i = 0; i < numRetValues; ++i) {
      // obtain ret instruction
      auto curRet = dynamic_cast<RetNode *>(CalleeCopy.getEnd()->operand(i));
      assert(curRet);

      auto curVal = curRet->getRetValue();
      RetValue->setVal(i, curVal);

      // replace ret with jmp
      auto curJmp = F.create<JmpNode>(curRet->getInputCF());
      RetRegion->addCFInput(curJmp);

      curRet->detach();
    }

    // expected that call has only one successor
    auto finalJmp = F.create<JmpNode>(RetRegion);
    bool done = false;
    for (auto &&U: CN.users()) {
      if (isa<RegionNode>(U)) {
        for (size_t i = 0; i < U->opCount(); ++i) {
          if (U->operand(i) == &CN) {
            U->setOperand(i, finalJmp);
            done = true;
            break;
          }
        }
      }
    }
    assert(done);
    CN.replaceWith(RetValue);

    // 3. move inlined nodes to Callee function
    CalleeCopy.getStart()->replaceWith(CN.getCVInput());
    for (auto &&node: CalleeCopy) {
      if (!isa<RetNode, FunctionArgNode>(node)) {
        F.addNode(std::move(node));
      }
    }
    CN.detach();
  }
};

} // namespace son