#include "gtest/gtest.h"

#include <Function.hpp>
using namespace son;

TEST(isa, isa) {
  FunctionType fnTy(ValueType::Void, {});
  Function F("Test_isa", fnTy);

  EXPECT_TRUE(isa<RegionNode>(*F.createNode<RegionNode>()));
  EXPECT_FALSE(isa<IfNode>(*F.createNode<RegionNode>()));
}

TEST(Region, phi) {
  FunctionType fnTy(ValueType::Void, {});
  Function F("Test_isa", fnTy);

  auto r = F.createNode<RegionNode>();
  auto phi1 = F.createNode<PhiNode>(r, 1, ValueType::Int32);
  auto phi2 = F.createNode<PhiNode>(r, 1, ValueType::Int32);

  auto &&phis = r->phis();
  EXPECT_EQ(phis.size(), 2);
  // EXPECT_EQ(phis.count(phi1), 1);
  // EXPECT_EQ(phis.at(1), phi2);
}
