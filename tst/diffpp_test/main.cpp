#include <iostream>
#include <string>
#include <vector>

#include <diffpp/diffpp.hpp>

//////////////////////////////////////////////////////////////////////////////////////////
/// all tests shall be contained in functions with the following signature: 
typedef  bool(*test_function)();
/// a test returns true on success, and false on failure
//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////
/// test the difference between the strings "abcabba" and "cbabac". Expected result is 5.  
bool test_difference_strings( void );


//////////////////////////////////////////////////////////////////////////////////////////
/// compilation focused tests of dcontour_range_parser
bool test_dcountour_parser( void );


//////////////////////////////////////////////////////////////////////////////////////////
/// after definition, all test functions will be added to this array, for automated execution
test_function tests[] = {
  &test_difference_strings,   
  &test_dcountour_parser
};



bool test_difference_strings( void ) {
  const std::string abcabba("abcabba");
  const std::string cbabac("cbabac");
  
  /**/
  int result = diffpp::difference( abcabba.begin(), cbabac.begin(), int(abcabba.size()), int(cbabac.size()), std::equal_to<char>() );
  if ( result != 5 ) return false;

  result = diffpp::difference_bwd(abcabba.begin(), cbabac.begin(), abcabba.size(), cbabac.size(), std::equal_to<char>() );
  if ( result != 5 ) return false;

  
  /* sizeB=0 */
  result = diffpp::difference(std::string::iterator(), cbabac.begin(), 0, int(cbabac.size()), std::equal_to<char>() );
  if ( result != cbabac.size() ) return false;
  
  result = diffpp::difference_bwd(std::string::iterator(), cbabac.begin(), 0, int(cbabac.size()), std::equal_to<char>() );
  if ( result != cbabac.size() ) return false;

  /* sizeA=0 */
  result = diffpp::difference(abcabba.begin(), std::string::iterator(), int(abcabba.size()), 0, std::equal_to<char>() );
  if ( result != abcabba.size() ) return false;
  
  result = diffpp::difference_bwd(abcabba.begin(), std::string::iterator(), int(abcabba.size()), 0, std::equal_to<char>() );
  if ( result != abcabba.size() ) return false;

  /* A == B */
  result = diffpp::difference( abcabba.begin(), abcabba.begin(), int(abcabba.size()), int(abcabba.size()), std::equal_to<char>() );
  if ( result != 0 ) return false;

  result = diffpp::difference_bwd(abcabba.begin(), abcabba.begin(), abcabba.size(), abcabba.size(), std::equal_to<char>() );
  if ( result != 0 ) return false;

  /* N+M = 0 */
  result = diffpp::difference( std::string::iterator(), std::string::iterator(), 0, 0, std::equal_to<char>() );
  if ( result != 0 ) return false;

  result = diffpp::difference_bwd(std::string::iterator(), std::string::iterator(), 0, 0, std::equal_to<char>() );
  if ( result != 0 ) return false;

  return true;
}

bool test_dcountour_parser( void ) { 
  typedef diffpp::algorithms::detail::greedy_graph_walker< 
            diffpp::algorithms::detail::forward_direction_policy,
            diffpp::algorithms::detail::fwd_monitored_kdmap_stdmap > computer_type;
  computer_type computer;
  
  std::string abcabba("abcabba");
  std::string cbabac("cbabac");

  int difference = computer(abcabba.begin(), cbabac.begin(), abcabba.size(), cbabac.size(), std::equal_to<char>());

  struct dummy{};
  diffpp::algorithms::detail::shortest_path_stepper< diffpp::algorithms::detail::forward_direction_policy, dummy > gen;
  diffpp::algorithms::detail::shortest_path_walker<diffpp::algorithms::detail::forward_direction_policy> parser;
  parser( abcabba.size(), cbabac.size(), computer, gen );

  return true;
}

int main() {
  const unsigned test_count = sizeof(tests) / sizeof(test_function);

  unsigned test_idx     = 0;
  unsigned passed_tests = 0;
  test_function test = tests[0];

  if ( test_count > 0 ) 
    do {
      bool test_result = (*test)();
      std::cout<< "test " << test_idx << " of " << test_count << "  " << ( test_result ? std::string("[passed]") : std::string("[failed]") ) << std::endl;
      passed_tests += test_result;
      ++test_idx;test = tests[test_idx];
    } while ( test_idx < test_count );

  std::cout << "Test done. Failed " << test_count - passed_tests << " out of " << test_count << std::endl;

  return ( test_count - passed_tests ) != 0;
}

