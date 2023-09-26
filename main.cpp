
#include "src/Instrs.hpp"

#include <iostream>

int main() {
  auto *EntryR = new son::Region();

  auto *Cnst_1 = new son::ConstInt32(1);
  auto *Cnst_5 = new son::ConstInt32(5);

  auto *Add_0 = new son::BinOp(son::BinOpIntrs::ADD, son::Type::Int32, Cnst_1,
                               Cnst_5, EntryR);

  auto *Ret = new son::ReturnNode(EntryR, Add_0);

  return 0;
}