#include "gtest/gtest.h"

#include "graphs.hpp"

#include <Analyses/DomTree.hpp>

using namespace son;

TEST(DomTree, example1) {
  BUILD_GRAPH_TEST1(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  EXPECT_TRUE(Func.verify());

  const std::vector<RegionNodeBase *> Nodes = {A, B, C, D,
                                                  E, F, G};
  for (auto && node : Nodes) {
    EXPECT_TRUE(DT.dominates(node, node));
    EXPECT_TRUE(DT.dominates(A, node));
  }
  EXPECT_FALSE(DT.dominates(F, D));
  EXPECT_FALSE(DT.dominates(E, D));
  EXPECT_FALSE(DT.dominates(G, D));
  EXPECT_FALSE(DT.dominates(C, D));

  EXPECT_TRUE(DT.dominates(B, D));
  EXPECT_TRUE(DT.dominates(B, C));
  EXPECT_TRUE(DT.dominates(B, G));
  EXPECT_TRUE(DT.dominates(B, E));
  EXPECT_TRUE(DT.dominates(B, F));

  EXPECT_FALSE(DT.dominates(D, B));
  EXPECT_FALSE(DT.dominates(C, B));
  EXPECT_FALSE(DT.dominates(G, B));
  EXPECT_FALSE(DT.dominates(E, B));
  EXPECT_FALSE(DT.dominates(F, B));
}

TEST(DomTree, example2) {
  BUILD_GRAPH_TEST2(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  EXPECT_TRUE(Func.verify());

  const std::vector<RegionNodeBase *> Nodes = {A, B, C, D, E, F,
                                                  G, H, I, K, J};
  for (auto &&node : Nodes) {
    EXPECT_TRUE(DT.dominates(node, node));
    EXPECT_TRUE(DT.dominates(A, node));
  }
  EXPECT_TRUE(DT.dominates(B, J));
  EXPECT_TRUE(DT.dominates(B, C));
  EXPECT_TRUE(DT.dominates(G, I));
  EXPECT_TRUE(DT.dominates(F, G));
  EXPECT_TRUE(DT.dominates(D, H));

  EXPECT_FALSE(DT.dominates(J, C));
  EXPECT_FALSE(DT.dominates(F, E));
}

TEST(DomTree, example3) {
  BUILD_GRAPH_TEST3(Func);
  EXPECT_TRUE(Func.verify());

  DomTree DT(Func);
  EXPECT_TRUE(Func.verify());

  const std::vector<RegionNodeBase *> Nodes = {A, B, C, D, E, F, G, H, I};
  for (auto &&node : Nodes) {
    EXPECT_TRUE(DT.dominates(node, node));
    EXPECT_TRUE(DT.dominates(A, node));
  }
}