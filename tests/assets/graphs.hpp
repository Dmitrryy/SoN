#pragma once

namespace son {

//                 +-----+
//                 |  A  |
//                 +-----+
//                    |
//                    V
//                 +-----+
//       ----------|  B  |---------
//       |         +-----+        |
//       |                        |
//       V                        V
//     +-----+     +-----+     +-----+
//     |  C  |  ---|  E  |<----|  F  |
//     +-----+  |  +-----+     +-----+
//        |     |                 |
//        |     V                 |
//        |  +-----+     +-----+  |
//        +->|  D  |<----|  G  |<-+
//           +-----+     +-----+
// img src: https://github.com/Bryanskiy/MasterCourse.Compilers/tree/master
#define BUILD_GRAPH_TEST1(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void, {ValueType::Int32}));   \
                                                                               \
  auto A = NAME.getStart();                                                    \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
  auto D = NAME.getEnd();                                                      \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
                                                                               \
  /* B if*/                                                                    \
  auto BIf =                                                                   \
      NAME.create<IfNode>(B, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto BIfTrue = NAME.create<IfTrueNode>(BIf);                                 \
  auto BIfFalse = NAME.create<IfFalseNode>(BIf);                               \
  /* C*/                                                                       \
  auto C = NAME.create<RegionNode>();                                          \
  C->addCFInput(BIfTrue);                                                      \
  auto CJmp = NAME.create<JmpNode>(C);                                         \
  /* F if*/                                                                    \
  auto F = NAME.create<RegionNode>();                                          \
  F->addCFInput(BIfFalse);                                                     \
  auto FIf =                                                                   \
      NAME.create<IfNode>(F, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto FIfTrue = NAME.create<IfTrueNode>(FIf);                                 \
  auto FIfFalse = NAME.create<IfFalseNode>(FIf);                               \
  /* E*/                                                                       \
  auto E = NAME.create<RegionNode>();                                          \
  E->addCFInput(FIfTrue);                                                      \
  auto EJmp = NAME.create<JmpNode>(E);                                         \
  /* G*/                                                                       \
  auto G = NAME.create<RegionNode>();                                          \
  G->addCFInput(FIfFalse);                                                     \
  auto GJmp = NAME.create<JmpNode>(G);                                         \
                                                                               \
  D->addCFInput(CJmp, EJmp, GJmp);                                             \
  Function::NamesMapTy NAME##Names = {{A, "A"}, {B, "B"}, {C, "C"}, {D, "D"},  \
                                      {E, "E"}, {F, "F"}, {G, "G"}};

//                 +-----+
//                 |  A  |
//                 +-----+
//                    V
//                 +-----+      +-----+
//       +-------->|  B  |----->|  J  |
//       |         +-----+      +-----+
//       |            V            |
//       |         +-----+         |
//       |   +---->|  C  |<--------+
//       |   |     +-----+
//       |   |        V
//       |   |     +-----+
//       |   +-----|  D  |
//       |         +-----+
//       |            V
//       |         +-----+
//       |         |  E  |<--------+
//       |         +-----+         |
//       |            V            |
//       |         +-----+         |
//       |         |  F  |---------+
//       |         +-----+
//       |            V
//    +-----+      +-----+      +-----+
//    |  H  |<-----|  G  |----->|  I  |
//    +-----+      +-----+      +-----+
//                                 |
//                 +-----+         |
//                 |  K  |<--------+
//                 +-----+
// img src: https://github.com/Bryanskiy/MasterCourse.Compilers/tree/master
#define BUILD_GRAPH_TEST2(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void, {ValueType::Int32}));   \
                                                                               \
  auto A = NAME.getStart();                                                    \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
  auto K = NAME.getEnd();                                                      \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
                                                                               \
  /* B if*/                                                                    \
  auto BIf =                                                                   \
      NAME.create<IfNode>(B, NAME.create<ConstantNode>(ValueType::Int1, 0));   \
  auto BIfTrue = NAME.create<IfTrueNode>(BIf);                                 \
  auto BIfFalse = NAME.create<IfFalseNode>(BIf);                               \
                                                                               \
  /* J*/                                                                       \
  auto J = NAME.create<RegionNode>();                                          \
  J->addCFInput(BIfFalse);                                                     \
  auto JJmp = NAME.create<JmpNode>(J);                                         \
                                                                               \
  /* C*/                                                                       \
  auto C = NAME.create<RegionNode>();                                          \
  auto CJmp = NAME.create<JmpNode>(C);                                         \
  C->addCFInput(BIfTrue, JJmp);                                                \
  /* D*/                                                                       \
  auto D = NAME.create<RegionNode>();                                          \
  D->addCFInput(CJmp);                                                         \
  auto DIf =                                                                   \
      NAME.create<IfNode>(D, NAME.create<ConstantNode>(ValueType::Int1, 0));   \
  auto DIfTrue = NAME.create<IfTrueNode>(DIf);                                 \
  auto DIfFalse = NAME.create<IfFalseNode>(DIf);                               \
  C->addCFInput(DIfTrue);                                                      \
  /* E*/                                                                       \
  auto E = NAME.create<RegionNode>();                                          \
  auto EJmp = NAME.create<JmpNode>(E);                                         \
  E->addCFInput(DIfFalse);                                                     \
  /* F*/                                                                       \
  auto F = NAME.create<RegionNode>();                                          \
  F->addCFInput(EJmp);                                                         \
  auto FIf =                                                                   \
      NAME.create<IfNode>(F, NAME.create<ConstantNode>(ValueType::Int1, 0));   \
  auto FIfTrue = NAME.create<IfTrueNode>(FIf);                                 \
  auto FIfFalse = NAME.create<IfFalseNode>(FIf);                               \
  E->addCFInput(FIfFalse);                                                     \
  /* G*/                                                                       \
  auto G = NAME.create<RegionNode>();                                          \
  G->addCFInput(FIfTrue);                                                      \
  auto GIf =                                                                   \
      NAME.create<IfNode>(G, NAME.create<ConstantNode>(ValueType::Int1, 0));   \
  auto GIfTrue = NAME.create<IfTrueNode>(GIf);                                 \
  auto GIfFalse = NAME.create<IfFalseNode>(GIf);                               \
  /* H*/                                                                       \
  auto H = NAME.create<RegionNode>();                                          \
  H->addCFInput(GIfTrue);                                                      \
  auto HJmp = NAME.create<JmpNode>(H);                                         \
  B->addCFInput(HJmp);                                                         \
  /* I*/                                                                       \
  auto I = NAME.create<RegionNode>();                                          \
  I->addCFInput(GIfFalse);                                                     \
  auto IJmp = NAME.create<JmpNode>(I);                                         \
  K->addCFInput(IJmp);                                                         \
  Function::NamesMapTy NAME##Names = {{A, "A"}, {B, "B"}, {C, "C"}, {D, "D"},  \
                                      {E, "E"}, {F, "F"}, {G, "G"}, {J, "J"},  \
                                      {H, "H"}, {K, "K"}, {I, "I"}};

//  +-----------------------+
//  |                       |
//  |              +-----+  |
//  |              |  A  |  |
//  |              +-----+  |
//  |                 V     |
//  |              +-----+  |
//  |    +---------|  B  |<-+
//  |    |         +-----+
//  |    V            V
//  | +-----+      +-----+
//  | |  E  |--+   |  C  |<--+
//  | +-----+  |   +-----+   |
//  |    V     |      V      |
//  | +-----+  |   +-----+   |
//  +-|  F  |  +-->|  D  |   |
//    +-----+      +-----+   |
//       V            V      |
//    +-----+      +-----+   |
//    |  H  |----->|  G  |---+
//    +-----+      +-----+
//       |            V
//       |         +-----+
//       +-------->|  I  |
//                 +-----+
// img src: https://github.com/Bryanskiy/MasterCourse.Compilers/tree/master
#define BUILD_GRAPH_TEST3(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void, {ValueType::Int32}));   \
                                                                               \
  auto A = NAME.getStart();                                                    \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
  auto I = NAME.getEnd();                                                      \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
                                                                               \
  /* B if*/                                                                    \
  auto BIf =                                                                   \
      NAME.create<IfNode>(B, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto BIfTrue = NAME.create<IfTrueNode>(BIf);                                 \
  auto BIfFalse = NAME.create<IfFalseNode>(BIf);                               \
  /* C*/                                                                       \
  auto C = NAME.create<RegionNode>();                                          \
  C->addCFInput(BIfTrue);                                                      \
  auto CJmp = NAME.create<JmpNode>(C);                                         \
  /* E if*/                                                                    \
  auto E = NAME.create<RegionNode>();                                          \
  E->addCFInput(BIfFalse);                                                     \
  auto EIf =                                                                   \
      NAME.create<IfNode>(E, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto EIfTrue = NAME.create<IfTrueNode>(EIf);                                 \
  auto EIfFalse = NAME.create<IfFalseNode>(EIf);                               \
  /* D*/                                                                       \
  auto D = NAME.create<RegionNode>();                                          \
  D->addCFInput(EIfFalse, CJmp);                                               \
  auto DJmp = NAME.create<JmpNode>(D);                                         \
  /* F if*/                                                                    \
  auto F = NAME.create<RegionNode>();                                          \
  F->addCFInput(EIfTrue);                                                      \
  auto FIf =                                                                   \
      NAME.create<IfNode>(F, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto FIfTrue = NAME.create<IfTrueNode>(FIf);                                 \
  auto FIfFalse = NAME.create<IfFalseNode>(FIf);                               \
  B->addCFInput(FIfTrue);                                                      \
  /* H if*/                                                                    \
  auto H = NAME.create<RegionNode>();                                          \
  H->addCFInput(FIfFalse);                                                     \
  auto HIf =                                                                   \
      NAME.create<IfNode>(H, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto HIfTrue = NAME.create<IfTrueNode>(HIf);                                 \
  auto HIfFalse = NAME.create<IfFalseNode>(HIf);                               \
  /* G if*/                                                                    \
  auto G = NAME.create<RegionNode>();                                          \
  G->addCFInput(HIfFalse, DJmp);                                               \
  auto GIf =                                                                   \
      NAME.create<IfNode>(G, NAME.create<ConstantNode>(ValueType::Int1, 1));   \
  auto GIfTrue = NAME.create<IfTrueNode>(GIf);                                 \
  auto GIfFalse = NAME.create<IfFalseNode>(GIf);                               \
  C->addCFInput(GIfFalse);                                                     \
                                                                               \
  I->addCFInput(HIfTrue, GIfTrue);                                             \
  Function::NamesMapTy NAME##Names = {{A, "A"}, {B, "B"}, {C, "C"},            \
                                      {D, "D"}, {E, "E"}, {F, "F"},            \
                                      {G, "G"}, {H, "H"}, {I, "I"}};

//
//  +------------------+
//  |                  |
//  |  +-----+      +-----+
//  |  |  A  |      |  D  |
//  |  +-----+      +-----+
//  |     |            ^
//  |     V            |
//  |  +-----+      +-----+
//  +->|  B  |----->|  C  |
//     +-----+      +-----+
//        |
//        |
//        V
//     +-----+
//     |  E  |
//     +-----+
//
#define BUILD_GRAPH_TEST4(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void,                         \
                                      {ValueType::Int1, ValueType::Int1}));    \
                                                                               \
  auto A = NAME.getStart();                                                    \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
  auto E = NAME.getEnd();                                                      \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
                                                                               \
  /* B if*/                                                                    \
  auto BIf = NAME.create<IfNode>(B, NAME.getArg(0));                           \
  auto BIfTrue = NAME.create<IfTrueNode>(BIf);                                 \
  auto BIfFalse = NAME.create<IfFalseNode>(BIf);                               \
  /* C*/                                                                       \
  auto C = NAME.create<RegionNode>();                                          \
  C->addCFInput(BIfTrue);                                                      \
  auto CJmp = NAME.create<JmpNode>(C);                                         \
  /* D */                                                                      \
  auto D = NAME.create<RegionNode>();                                          \
  D->addCFInput(CJmp);                                                         \
  auto DJmp = NAME.create<JmpNode>(D);                                         \
  B->addCFInput(DJmp);                                                         \
  /* E */                                                                      \
  E->addCFInput(BIfFalse);                                                     \
  Function::NamesMapTy NAME##Names = {                                         \
      {A, "A"}, {B, "B"}, {C, "C"}, {D, "D"}, {E, "E"}};

//                 +-----+
//                 |  A  |
//                 +-----+
//                    |
//                    V
//                 +-----+
//       +-------->|  B  |--------+
//       |         +-----+        |
//       |                        |
//       |                        V
//     +-----+     +-----+     +-----+
//     |  C  |<----|  E  |<----|  F  |
//     +-----+     +-----+     +-----+
//                    |           |
//                    V           |
//                 +-----+        |
//                 |  D  |<-------+
//                 +-----+
//
#define BUILD_GRAPH_TEST5(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void,                         \
                                      {ValueType::Int1, ValueType::Int1}));    \
                                                                               \
  auto A = NAME.getStart();                                                    \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
  auto D = NAME.getEnd();                                                      \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
  auto BJmp = NAME.create<JmpNode>(B);                                         \
                                                                               \
  /* F if */                                                                   \
  auto F = NAME.create<RegionNode>();                                          \
  F->addCFInput(BJmp);                                                         \
  auto FIf = NAME.create<IfNode>(F, NAME.getArg(0));                           \
  auto FIfTrue = NAME.create<IfTrueNode>(FIf);                                 \
  auto FIfFalse = NAME.create<IfFalseNode>(FIf);                               \
  /* E if */                                                                   \
  auto E = NAME.create<RegionNode>();                                          \
  E->addCFInput(FIfFalse);                                                     \
  auto EIf = NAME.create<IfNode>(E, NAME.getArg(1));                           \
  auto EIfTrue = NAME.create<IfTrueNode>(EIf);                                 \
  auto EIfFalse = NAME.create<IfFalseNode>(EIf);                               \
  /* C */                                                                      \
  auto C = NAME.create<RegionNode>();                                          \
  C->addCFInput(EIfTrue);                                                      \
  auto CJmp = NAME.create<JmpNode>(C);                                         \
  B->addCFInput(CJmp);                                                         \
                                                                               \
  D->addCFInput(EIfFalse, FIfTrue);                                            \
  Function::NamesMapTy NAME##Names = {{A, "A"}, {B, "B"}, {C, "C"},            \
                                      {D, "D"}, {E, "E"}, {F, "F"}};

//                 +-----+
//                 |Entry|
//                 +-----+
//                    V
//                 +-----+
//    +----------->|  A  |
//    |            +-----+
//    |               V
//    |            +-----+          +-----+
//    |      +---->|  B  |--------->|  J  |
//    |      |     +-----+          +-----+
//    |      |        V                |
//    |      |     +-----+    +-----+  |
//    |      |     |  C  |--->|EX(G)|  |
//    |      |     +-----+    +-----+  |
//    |      |        V                |
//    |      |     +-----+             |
//    |      |     |  D  |<------------+
//    |      |     +-----+
//    |      |        V
//    |      |     +-----+
//    |      +-----|  E  |
//    |            +-----+
//    |               V
//    |            +-----+
//    +------------|  F  |
//                 +-----+
//
#define BUILD_GRAPH_TEST6(NAME)                                                \
  Function NAME("Hello", FunctionType(ValueType::Void,                         \
                                      {ValueType::Int1, ValueType::Int1,       \
                                       ValueType::Int1, ValueType::Int1}));    \
                                                                               \
  auto Entry = NAME.getStart();                                                \
  auto EntryJmp = NAME.create<JmpNode>(Entry);                                 \
  auto G = NAME.getEnd();                                                      \
  auto A = NAME.create<RegionNode>();                                          \
  A->addCFInput(EntryJmp);                                                     \
  auto AJmp = NAME.create<JmpNode>(A);                                         \
                                                                               \
  /* B if */                                                                   \
  auto B = NAME.create<RegionNode>();                                          \
  B->addCFInput(AJmp);                                                         \
  auto BIf = NAME.create<IfNode>(B, NAME.getArg(0));                           \
  auto BIfTrue = NAME.create<IfTrueNode>(BIf);                                 \
  auto BIfFalse = NAME.create<IfFalseNode>(BIf);                               \
  /* C if */                                                                   \
  auto C = NAME.create<RegionNode>();                                          \
  C->addCFInput(BIfFalse);                                                     \
  auto CIf = NAME.create<IfNode>(C, NAME.getArg(1));                           \
  auto CIfTrue = NAME.create<IfTrueNode>(CIf);                                 \
  auto CIfFalse = NAME.create<IfFalseNode>(CIf);                               \
  G->addCFInput(CIfFalse);                                                     \
  /* J */                                                                      \
  auto J = NAME.create<RegionNode>();                                          \
  J->addCFInput(BIfTrue);                                                      \
  auto JJmp = NAME.create<JmpNode>(J);                                         \
  /* D */                                                                      \
  auto D = NAME.create<RegionNode>();                                          \
  D->addCFInput(CIfTrue, JJmp);                                                \
  auto DJmp = NAME.create<JmpNode>(D);                                         \
  /* E if */                                                                   \
  auto E = NAME.create<RegionNode>();                                          \
  E->addCFInput(DJmp);                                                         \
  auto EIf = NAME.create<IfNode>(E, NAME.getArg(2));                           \
  auto EIfTrue = NAME.create<IfTrueNode>(EIf);                                 \
  auto EIfFalse = NAME.create<IfFalseNode>(EIf);                               \
  B->addCFInput(EIfTrue);                                                      \
  /* F */                                                                      \
  auto F = NAME.create<RegionNode>();                                          \
  F->addCFInput(EIfFalse);                                                     \
  auto FJmp = NAME.create<JmpNode>(F);                                         \
  A->addCFInput(FJmp);                                                         \
                                                                               \
  Function::NamesMapTy NAME##Names = {{A, "A"}, {B, "B"}, {C, "C"}, {D, "D"},  \
                                      {E, "E"}, {F, "F"}, {G, "G"}, {J, "J"}};

} // namespace son