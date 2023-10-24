#pragma once

#include <string>

namespace son {

enum class NodeType {
#define NODE_OPCODE_DEFINE(opc_name) opc_name,
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
  LastOpcode
};

constexpr auto getOpcName(NodeType opc) {
  switch (opc) {
#define NODE_OPCODE_DEFINE(opc_name)                                           \
  case NodeType::opc_name:                                                   \
    return #opc_name;
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
  }
}

enum class ValueType { Void, Int1, Int8, Int32, Int64, Ptr };

} // namespace son