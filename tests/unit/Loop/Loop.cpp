#include "gtest/gtest.h"

#include "graphs.hpp"

#include <Analyses/Loop.hpp>

using namespace son;

TEST(Loop, example1) {
  BUILD_GRAPH_TEST1(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  auto &&dfsResult = Func.dfs();

  auto &&LI = LoopInfo::loopAnalyze(Func, dfsResult, DT);
  EXPECT_TRUE(Func.verify());

  EXPECT_EQ(LI.loopsCount(), 0);

  EXPECT_EQ(LI.getLoopFor(A), nullptr);
  EXPECT_EQ(LI.getLoopFor(B), nullptr);
  EXPECT_EQ(LI.getLoopFor(C), nullptr);
  EXPECT_EQ(LI.getLoopFor(D), nullptr);
  EXPECT_EQ(LI.getLoopFor(E), nullptr);
  EXPECT_EQ(LI.getLoopFor(F), nullptr);
  EXPECT_EQ(LI.getLoopFor(G), nullptr);
}

TEST(Loop, example2) {
  BUILD_GRAPH_TEST2(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  auto &&dfsResult = Func.dfs();

  auto &&LI = LoopInfo::loopAnalyze(Func, dfsResult, DT);
  EXPECT_TRUE(Func.verify());

  EXPECT_EQ(LI.loopsCount(), 3);

  // without loops
  EXPECT_EQ(LI.getLoopFor(A), nullptr);
  EXPECT_EQ(LI.getLoopFor(I), nullptr);
  EXPECT_EQ(LI.getLoopFor(K), nullptr);

  // B loop
  auto BLoop = LI.getLoopFor(B);
  ASSERT_NE(BLoop, nullptr);
  EXPECT_EQ(BLoop->header(), B);

  const auto &BBody = BLoop->blocks();
  EXPECT_EQ(BBody.size(), 4);
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), B) != BBody.end());
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), J) != BBody.end());
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), G) != BBody.end());
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), H) != BBody.end());

  const auto &BLatches = BLoop->latches();
  EXPECT_EQ(BLatches.size(), 1);
  EXPECT_TRUE(std::find(BLatches.begin(), BLatches.end(), H) != BLatches.end());

  // C loop
  auto CLoop = LI.getLoopFor(C);
  ASSERT_NE(CLoop, nullptr);
  EXPECT_EQ(CLoop->header(), C);

  const auto &CBody = CLoop->blocks();
  EXPECT_EQ(CBody.size(), 2);
  EXPECT_TRUE(std::find(CBody.begin(), CBody.end(), C) != CBody.end());
  EXPECT_TRUE(std::find(CBody.begin(), CBody.end(), D) != CBody.end());

  const auto &CLatches = CLoop->latches();
  EXPECT_EQ(CLatches.size(), 1);
  EXPECT_TRUE(std::find(CLatches.begin(), CLatches.end(), D) != CLatches.end());

  // E loop
  auto ELoop = LI.getLoopFor(E);
  ASSERT_NE(ELoop, nullptr);
  EXPECT_EQ(ELoop->header(), E);

  const auto &EBody = ELoop->blocks();
  EXPECT_EQ(EBody.size(), 2);
  EXPECT_TRUE(std::find(EBody.begin(), EBody.end(), E) != EBody.end());
  EXPECT_TRUE(std::find(EBody.begin(), EBody.end(), F) != EBody.end());

  const auto &ELatches = ELoop->latches();
  EXPECT_EQ(ELatches.size(), 1);
  EXPECT_TRUE(std::find(ELatches.begin(), ELatches.end(), F) != ELatches.end());

  // check region to loop map
  EXPECT_EQ(LI.getLoopFor(B), BLoop);
  EXPECT_EQ(LI.getLoopFor(J), BLoop);
  EXPECT_EQ(LI.getLoopFor(G), BLoop);
  EXPECT_EQ(LI.getLoopFor(H), BLoop);

  EXPECT_EQ(LI.getLoopFor(C), CLoop);
  EXPECT_EQ(LI.getLoopFor(D), CLoop);

  EXPECT_EQ(LI.getLoopFor(E), ELoop);
  EXPECT_EQ(LI.getLoopFor(F), ELoop);

  // loop nesting
  const auto &Top = LI.topLoops();
  EXPECT_EQ(Top.size(), 1);
  EXPECT_TRUE(std::find(Top.begin(), Top.end(), BLoop) != Top.end());

  const auto &BSub = BLoop->subloops();
  EXPECT_EQ(BSub.size(), 2);
  EXPECT_TRUE(std::find(BSub.begin(), BSub.end(), CLoop) != BSub.end());
  EXPECT_TRUE(std::find(BSub.begin(), BSub.end(), ELoop) != BSub.end());

  const auto &CSub = CLoop->subloops();
  EXPECT_EQ(CSub.size(), 0);

  const auto &ESub = ELoop->subloops();
  EXPECT_EQ(ESub.size(), 0);
}

TEST(Loop, example3) {
  BUILD_GRAPH_TEST3(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  auto &&dfsResult = Func.dfs();

  auto &&LI = LoopInfo::loopAnalyze(Func, dfsResult, DT);
  EXPECT_TRUE(Func.verify());
  EXPECT_EQ(LI.loopsCount(), 1);

  // without loops
  EXPECT_EQ(LI.getLoopFor(A), nullptr);
  EXPECT_EQ(LI.getLoopFor(I), nullptr);

  auto BLoop = LI.getLoopFor(B);
  ASSERT_NE(BLoop, nullptr);
  EXPECT_EQ(BLoop->header(), B);

  const auto &BBody = BLoop->blocks();
  EXPECT_EQ(BBody.size(), 3);
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), B) != BBody.end());
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), E) != BBody.end());
  EXPECT_TRUE(std::find(BBody.begin(), BBody.end(), F) != BBody.end());

  const auto &BLatches = BLoop->latches();
  EXPECT_EQ(BLatches.size(), 1);
  EXPECT_TRUE(std::find(BLatches.begin(), BLatches.end(), F) != BLatches.end());

  // check map
  EXPECT_EQ(LI.getLoopFor(B), BLoop);
  EXPECT_EQ(LI.getLoopFor(E), BLoop);
  EXPECT_EQ(LI.getLoopFor(F), BLoop);
}