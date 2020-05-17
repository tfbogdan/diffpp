#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <diffpp/diffpp.hpp>

using namespace std::string_view_literals;

auto abcabba = "abcabba"sv;
auto cbabac = "cbabac"sv;

TEST(StrDiff, Basic) {
  auto result = diffpp::difference( abcabba.begin(), abcabba.end(), cbabac.begin(), cbabac.end());
  ASSERT_EQ(result, 5);
}

TEST(StrDiff, BasicButLong) {
  auto a = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum)"sv;
  auto b = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum)"sv;
  auto result = diffpp::difference( a.begin(), a.end(), b.begin(), b.end());
  ASSERT_EQ(result, 0);
}

TEST(StrDiff, NoCommonSubSequence) {
  auto a = "abc"sv;
  auto b = "def"sv;
  auto result = diffpp::difference( a.begin(), a.end(), b.begin(), b.end());
  ASSERT_EQ(result, 6);
}



TEST(StrDiff, NullLHS) {
  /* sizeB=0 */
  std::string nullStr;
  auto result = diffpp::difference(nullStr.begin(), nullStr.end(), cbabac.begin(), cbabac.end());
  ASSERT_EQ(cbabac.size(), result);
}

TEST(StrDiff, NullRHS) {
  auto result = diffpp::difference(abcabba.begin(), abcabba.end(), (char*)nullptr, (char*)nullptr);
  ASSERT_EQ(abcabba.size(), result);
}

TEST(StrDiff, EqualOperands) {
  auto result = diffpp::difference( abcabba.begin(), abcabba.end(), abcabba.begin(), abcabba.end());
  ASSERT_EQ(result, 0);
}

TEST(StrDiff, BothEmpty) {
  auto result = diffpp::difference( (char*)nullptr, (char*)nullptr, (char*)nullptr, (char*)nullptr);
  ASSERT_EQ(result, 0);
}
