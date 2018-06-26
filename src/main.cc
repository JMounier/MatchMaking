#include "match.hh"

#include <iostream>
#include <string>

static void error_msg(const std::string& loc, const std::string& str = "")
{
    std::cerr << "usage: " << loc << " <nb_team> <nb_field>" << std::endl << str;
    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 3)
        error_msg(argv[0]);

    int nb_team = std::atoi(argv[1]);
    int nb_field = std::atoi(argv[2]);

    std::cerr << argv[1] << "=" << nb_team << std::endl;
    std::cerr << argv[2] << "=" << nb_field << std::endl;

    if (nb_field < 1 || nb_team < 3)
        error_msg(argv[0],"invalid value\n");

    std::cout << make_match(nb_team, nb_field);

    return 0;
}
