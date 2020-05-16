#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <diffpp/diffpp.hpp>

using namespace std::string_view_literals;

auto abcabba = "abcabba"sv;
auto cbabac = "cbabac"sv;

TEST(StrDiff, BasicForward) {
  auto result = diffpp::difference( abcabba.begin(), cbabac.begin(), int(abcabba.size()), int(cbabac.size()), std::equal_to<char>() );
  ASSERT_EQ(result, 5);
}

TEST(StrDiff, BasicReverse) {
  auto result = diffpp::difference_bwd(abcabba.begin(), cbabac.begin(), abcabba.size(), cbabac.size(), std::equal_to<char>() );
  ASSERT_EQ(result, 5);
}

TEST(StrDiff, NullLHS) {
  /* sizeB=0 */
  auto result = diffpp::difference(std::string::iterator(), cbabac.begin(), 0, int(cbabac.size()), std::equal_to<char>() );
  ASSERT_EQ(cbabac.size(), result);
}

TEST(StrDiff, NullLHSReverse) {
  auto result = diffpp::difference_bwd(std::string::iterator(), cbabac.begin(), 0, int(cbabac.size()), std::equal_to<char>() );
  ASSERT_EQ(cbabac.size(), result);
}

TEST(StrDiff, NullRHS) {
  auto result = diffpp::difference(abcabba.begin(), std::string::iterator(), int(abcabba.size()), 0, std::equal_to<char>() );
  ASSERT_EQ(abcabba.size(), result);
}

TEST(StrDiff, NullRHSReverse) {
  auto result = diffpp::difference_bwd(abcabba.begin(), std::string::iterator(), int(abcabba.size()), 0, std::equal_to<char>() );
  ASSERT_EQ(abcabba.size(), result);
}

TEST(StrDiff, EqualOperands) {
  /* A == B */
  auto result = diffpp::difference( abcabba.begin(), abcabba.begin(), int(abcabba.size()), int(abcabba.size()), std::equal_to<char>() );
  ASSERT_EQ(result, 0);
}

TEST(StrDiff, EqualOperandsReverse) {
  auto result = diffpp::difference_bwd(abcabba.begin(), abcabba.begin(), abcabba.size(), abcabba.size(), std::equal_to<char>() );
  ASSERT_EQ(result, 0);
}

TEST(StrDiff, BothEmpty) {
  /* N+M = 0 */
  auto result = diffpp::difference( std::string::iterator(), std::string::iterator(), 0, 0, std::equal_to<char>() );
  ASSERT_EQ(result, 0);
}

TEST(StrDiff, BothEmptyReverse) {
  auto result = diffpp::difference_bwd(std::string::iterator(), std::string::iterator(), 0, 0, std::equal_to<char>() );
  ASSERT_EQ(result, 0);
}

TEST(DContour, Parser) {
  typedef diffpp::algorithms::detail::greedy_graph_walker< 
            diffpp::algorithms::detail::forward_direction_policy,
            diffpp::algorithms::detail::fwd_monitored_kdmap_stdmap > computer_type;
  computer_type computer;
  
  std::string abcabba("abcabba");
  std::string cbabac("cbabac");

  struct dummy{};
  diffpp::algorithms::detail::shortest_path_stepper< diffpp::algorithms::detail::forward_direction_policy, dummy > gen;
  diffpp::algorithms::detail::shortest_path_walker<diffpp::algorithms::detail::forward_direction_policy> parser;
  parser( abcabba.size(), cbabac.size(), computer, gen );
}
