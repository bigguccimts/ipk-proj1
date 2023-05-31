/**
 * @file ipkcpc.hpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains declarations of utility functions
 */
#ifndef IPKCPC_HPP
#define IPKCPC_HPP

#include <string>
using namespace std;

int check_args(int argc, char **argv, string &h, string &p, string &m);

void print_help();

#endif // IPKCPC_HPP