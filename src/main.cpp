#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << "surge v0.1.0\n";
    std::cout << "Args Received: " << args.size() << "\n";

    for (const std::string& arg : args) {
        std::cout << "\t- " << arg << "\n";
    }

    return 0;
}