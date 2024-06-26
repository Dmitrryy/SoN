#pragma once

#include "Analyses/DomTree.hpp"
#include "Function.hpp"
#include "Node.hpp"

#include <vector>

namespace son {

class ChecksElimination {

public:
  void run(Function &F) {
    auto &&[order, parents, visited, rpo] = F.dfs();
    DomTree DT(F);

    _zeroCheckElimination(F, rpo, DT);
    _boundsCheckElimination(F, rpo, DT);
    F.clean();
  }

private:
  void _zeroCheckElimination(Function &F,
                             const std::vector<RegionNodeBase *> &rpo,
                             DomTree &DT) {
    // CallBuiltin is terminator for Region.
    for (auto Region : rpo) {
      auto Term = Region->terminator();
      if (auto *CN = dynamic_cast<CallBuiltinNode *>(Term)) {
        if (CN->getName() == "nullCheck") {
          auto obj = CN->operand(0);
          auto users = obj->users();
          for (auto U : users) {
            if (U == CN) {
              continue;
            }
            if (auto SecondCheck = dynamic_cast<CallBuiltinNode *>(U)) {
              if (SecondCheck->getName() == "nullCheck") {
                // check domination
                auto beforeRegion = SecondCheck->getCVInput();
                auto afterRegion = SecondCheck->getNextRegion();
                if (DT.dominates(Region, beforeRegion)) {
                  // remove dominated check
                  SecondCheck->detach();
                  auto Jmp = F.create<JmpNode>(beforeRegion);
                  afterRegion->setOperand(0, Jmp);
                }
              }
            }
          } // for end
        }
      }
    } // for end
  }   // _zeroCheckElimination

  void _boundsCheckElimination(Function &F,
                               const std::vector<RegionNodeBase *> &rpo,
                               DomTree &DT) {
    // CallBuiltin is terminator for Region.
    for (auto Region : rpo) {
      auto Term = Region->terminator();
      if (auto *CN = dynamic_cast<CallBuiltinNode *>(Term)) {
        if (CN->getName() == "boundsCheck") {
          auto obj = CN->operand(0);
          auto users = obj->users();
          const auto curBound =
              dynamic_cast<ConstantNode *>(CN->operand(1))->getConstant();
          for (auto U : users) {
            if (U == CN) {
              continue;
            }
            if (auto SecondCheck = dynamic_cast<CallBuiltinNode *>(U)) {
              if (SecondCheck->getName() == "boundsCheck") {
                // check domination and second argument
                const auto secondBound =
                    dynamic_cast<ConstantNode *>(SecondCheck->operand(1))
                        ->getConstant();
                auto beforeRegion = SecondCheck->getCVInput();
                auto afterRegion = SecondCheck->getNextRegion();
                if (secondBound <= curBound && DT.dominates(Region, beforeRegion)) {
                  // remove dominated check
                  SecondCheck->detach();
                  auto Jmp = F.create<JmpNode>(beforeRegion);
                  afterRegion->setOperand(0, Jmp);
                }
              }
            }
          } // for end
        }
      }
    } // for end
  }   // _boundsCheckElimination
};
} // namespace son