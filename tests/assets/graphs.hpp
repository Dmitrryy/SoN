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

} // namespace son