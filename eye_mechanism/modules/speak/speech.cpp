#include <string>
#include <cstdlib>

int main() {
    std::string cmd = "say -v Samantha -r 180 \"Hello, how can I help you?\"";
    system(cmd.c_str());
    return 0;
}