#include "Node.hpp"
#include "Function.hpp"
#include "Opcodes.hpp"

namespace son {

std::vector<PhiNode *> RegionNodeBase::phis() const {
  std::vector<PhiNode *> result;
  for (auto &&U : users()) {
    if (isa<PhiNode>(U)) {
      result.push_back(dynamic_cast<PhiNode *>(U));
    }
  }
  return result;
}

std::vector<RegionNodeBase *> RegionNodeBase::successors() const {
  if (isa<EndNode>(this)) {
    return {};
  }
  std::vector<RegionNodeBase *> result;
  auto Term = terminator();
  assert(Term);
  if (isa<JmpNode, RetNode>(Term)) {
    assert(Term->usersCount() == 1);
    auto *S = dynamic_cast<RegionNodeBase *>(*Term->users().begin());
    assert(S);
    result.push_back(S);
  } else if (isa<IfNode>(Term)) {
    assert(Term->usersCount() == 2);
    for (auto &&U : Term->users()) {
      assert((isa<IfTrueNode, IfFalseNode>(U)));
      assert(U->usersCount() == 1);
      auto S = dynamic_cast<RegionNodeBase *>(*U->users().begin());
      assert(S);
      result.push_back(S);
    }
  } else if (isa<CallNode, CallBuiltinNode>(Term)) {
    for (auto &&U : Term->users()) {
      if (isa<RegionNode>(U)) {
        result.push_back(dynamic_cast<RegionNode *>(U));
      }
    }
  }

  return result;
}

std::vector<RegionNodeBase *> RegionNodeBase::predecessors() const {
  std::vector<RegionNodeBase *> result;

  for (auto &&operand : operands()) {
    if (isa<JmpNode>(operand)) {
      auto *P = dynamic_cast<RegionNodeBase *>(operand->operand(0));
      assert(P);
      result.push_back(P);
    } else if (isa<IfTrueNode, IfFalseNode>(operand)) {
      assert(operand->opCount() == 1);
      auto *If = dynamic_cast<IfNode *>(operand->operand(0));
      assert(If);
      result.push_back(If->getInputCF());
    } else if (isa<CallNode>(operand)) {
      auto CN = dynamic_cast<CallNode *>(operand);
      result.push_back(CN->getCVInput());
    } else if (isa<CallBuiltinNode>(operand)) {
      auto CN = dynamic_cast<CallBuiltinNode *>(operand);
      result.push_back(CN->getCVInput());
    }
  }

  return result;
}

CFNode *RegionNodeBase::terminator() const {
  for (auto &&Operand : users()) {
    if (isa<RetNode, JmpNode, IfNode, CallNode, CallBuiltinNode>(Operand)) {
      return dynamic_cast<CFNode *>(Operand);
    }
  }
  return nullptr;
}

CallNode::CallNode(RegionNodeBase *cfInput, Function &Callee,
                   const std::vector<Node *> &args)
    : CFNode(NodeType::Call, Callee.getFnTy().retType, args.size() + 1),
      m_callee(&Callee) {
  for (size_t i = 0; i < args.size(); ++i) {
    assert(args[i]->valueTy() == Callee.getFnTy().argsTypes[i]);
    setOperand(i, args[i]);
  }
  setOperand(args.size(), cfInput);
}

void CallNode::dump(
    std::ostream &stream,
    const std::unordered_map<const Node *, std::string> &names) const /*override*/ {
  if (valueTy() != ValueType::Void) {
    stream << names.at(this) << " = " << getTyName(valueTy()) << ' ';
  }
  stream << getOpcName(nodeTy()) << " \"" << m_callee->getName() << "\"(";
  for (int i = 0; i < opCount() - 1; ++i) {
    auto opr = operand(i);
    stream << ((i == 0) ? "" : ", ");
    stream << getTyName(opr->valueTy()) << ' ' << names.at(opr);
  }
  stream << ") |-> " << names.at(getNextRegion());
}

} // namespace son