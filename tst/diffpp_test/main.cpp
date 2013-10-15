#include <iostream>



#include <string>

#include <vector>

#include <diffpp/diffpp.hpp>

std::string leftstr="ABCABBA";
std::string rightstr="CBABAC";

template < typename container_t > inline void print_diff_merge( const container_t &sln ) {
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
}

int main() {

    typedef std::vector< diffpp::edit > sln_t;
    std::cout<< "strA=";std::cin>>leftstr;
    std::cout<< "strB=";std::cin>>rightstr;

    sln_t sln;
    sln_t slnb;
		sln_t slnsl;

    auto pred = [](const char &a, const char &b) { return a == b; };
    diffpp::algorithms::diff_greedyfwd(std::begin(leftstr), std::end(leftstr), std::begin(rightstr), std::end(rightstr), sln, pred);
    diffpp::algorithms::diff_greedybwd(std::begin(leftstr), std::end(leftstr), std::begin(rightstr), std::end(rightstr), slnb, pred );
		diffpp::algorithms::diff_linear(std::begin(leftstr), std::end(leftstr), std::begin(rightstr), std::end(rightstr), slnsl, pred );

    print_diff_merge(sln);
    print_diff_merge(slnb);
		print_diff_merge(slnsl);

    return 0;
}

