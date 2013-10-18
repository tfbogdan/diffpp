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
/// after definition, all test functions will be added to this array, for automated execution
test_function tests[] = {
  &test_difference_strings,   
};



bool test_difference_strings( void ) {
  const std::string abcabba("abcabba");
  const std::string cbabac("cbabac");

  int result = diffpp::difference<std::string::const_iterator, std::string::const_iterator> ( abcabba.begin(), cbabac.begin(), static_cast<int>(abcabba.size()), static_cast<int>(cbabac.size()), std::equal_to<char>() );
  if ( result != 5 ) return false;

  result = diffpp::difference_bwd(abcabba.begin(), cbabac.begin(), abcabba.size(), cbabac.size(), std::equal_to<char>() );
  if ( result != 5 ) return false;

  
  result = diffpp::difference(std::string::iterator(), cbabac.begin(), 0, static_cast<int>(cbabac.size()), std::equal_to<char>() );
  

  return result == cbabac.size();
}

template < typename container_t, typename range_a, typename range_b > 
inline std::string print_diff_merge( const container_t &sln, const range_a &leftstr, const range_b &rightstr ) {
    std::string merged;
    for ( auto iter(sln.begin()); iter != sln.end(); ++iter ) {
        diffpp::point start = iter->get_start();
        diffpp::edit::command_t comm = iter->get_command();
        int dir = iter->get_direction();
        int matches = iter->get_matches();
        diffpp::point end = iter->get_end();

        int idxA = ~dir ? start.x : end.x;
        int idxB = ~dir ? start.y : end.y;
        

        if ( ~dir ) {
            if ( comm == diffpp::edit::DELETE && idxA >= 0 ) {
                    std::cout << leftstr[idxA] << " -> "<< leftstr[idxA]<< std::endl;
                    merged.push_back(leftstr[idxA]);
                    ++idxA;
            } else {
                if ( idxB >= 0 ) {
                    std::cout << "     " << rightstr[idxB] << " <- " << rightstr[idxB] << std::endl;
                    merged.push_back(rightstr[idxB]);
                }
                ++idxB;
            }
        }

        int step(0);
        for ( ; step < matches; ++step ) {
            char a = ' ';
            if ( idxA+step >= 0 && idxA+step < leftstr.size() ) {
                std::cout << leftstr[idxA+step]<< " -> ";
                a = leftstr[idxA+step];
            } else {
                std::cout << "     ";
                a = rightstr[idxB+step];
            }

            merged.push_back(a);
            std::cout << a;

            if ( idxB+step >= 0 && idxB+step < rightstr.size() ) {
                std::cout << " <- " << rightstr[idxB+step]<<std::endl;
            } else {
                std::cout << std::endl;
            }
        }

        if ( !(~dir) ) {
            if ( comm == diffpp::edit::DELETE && idxA+step < static_cast<int>(leftstr.size()) ) {
                std::cout<< leftstr[idxA+step] << " -> " << leftstr[idxA+step]<< std::endl;
                merged.push_back(leftstr[idxA+step]);
            } else if( idxB+step < static_cast<int>(rightstr.size()) ) {
                std::cout<< "     " << rightstr[idxB+step] << " <- " << rightstr[idxB+step] << std::endl;
                merged.push_back(rightstr[idxB+step]);
            }
        }
    }

    std::cout << "merge of:" << std::endl << "\t\"" << leftstr << "\"" << std::endl << "with:" << std::endl << "\t" << "\"" << rightstr << "\"" <<
                 std::endl << "results:" << std::endl << "\t\t\"" << merged << "\"" << std::endl;
		return merged;
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
      ++test_idx;
    } while ( test_idx < test_count );

  std::cout << "Test done. Failed " << test_count - passed_tests << " out of " << test_count << std::endl;

  return ( test_count - passed_tests ) != 0;
}

