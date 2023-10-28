#pragma once

#include <string>
#include <memory>
#include <cassert>

namespace son {

enum class ValueType { Void, Int1, Int8, Int32, Int64, Ptr };

enum class NodeType {
  Unknown,
#define NODE_OPCODE_ALL_DEFINE
#define NODE_OPCODE_DEFINE(opc_name) opc_name,
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_ALL_DEFINE
  LastOpcode
};

static std::string getOpcName(NodeType opc) {
  switch (opc) {
#define NODE_OPCODE_ALL_DEFINE
#define NODE_OPCODE_DEFINE(opc_name)                                           \
  case NodeType::opc_name:                                                     \
    return #opc_name;
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_ALL_DEFINE
  }
  return "UNKNOWN";
}

constexpr bool isRegionTerminator(NodeType opc) {
  switch (opc) {
#define NODE_OPCODE_TERMINATORS_DEFINE
#define NODE_OPCODE_DEFINE(opc_name) case NodeType::opc_name:
#include "Opcodes.def"
#undef NODE_OPCODE_DEFINE
#undef NODE_OPCODE_TERMINATORS_DEFINE
    return true;
  }
  return false;
}

template <typename To, typename From, typename Enabler = void> struct isa_impl {
  static inline bool doit(const From &Val) { return To::classof(&Val); }
};

// Always allow upcasts, and perform no dynamic check for them.
template <typename To, typename From>
struct isa_impl<To, From, std::enable_if_t<std::is_base_of_v<To, From>>> {
  static inline bool doit(const From &) { return true; }
};

template <typename To, typename From> struct isa_impl_cl {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From> {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From>
struct isa_impl_cl<To, const std::unique_ptr<From>> {
  static inline bool doit(const std::unique_ptr<From> &Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl_cl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From *> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From *const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From *> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From>
struct isa_impl_cl<To, const From *const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From>
[[nodiscard]] inline bool isa(const From &Val) {
  return isa_impl_cl<To, const From>::doit(Val);
}

template <typename First, typename Second, typename... Rest, typename From>
[[nodiscard]] inline bool isa(const From &Val) {
  return isa<First>(Val) || isa<Second, Rest...>(Val);
}

} // namespace son