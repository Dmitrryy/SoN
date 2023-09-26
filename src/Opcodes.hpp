#pragma once

namespace son {

enum class NodeType {
    UNKNOWN,
    BinOp,
    UnOp,
    Const,
    // Start,
    // End,
    // Ret,
    // Region,
    // IF,
    // JMP,
    Control,
    Region,
    PHI,
    Projection
};

enum class ControlNodeOp {
    UNKNOWN,
    // Region,
    Jmp,
    If,
    Return,
    Projection
};

enum class BinOpIntrs {
    UNKNOWN,
    ADD,
    SUB,
    DIV,
    MUL,
    REM,
    CMP,
    //
    AND,
    OR,
    XOR,
};

enum class UnOpInstrs {
    UNKNOWN,
    NOT,
    NEG,
};

enum class CmpOpInstrs {
    UNKNOWN,
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
};

enum class Type {
    Void,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Ptr
};

} // namespace son