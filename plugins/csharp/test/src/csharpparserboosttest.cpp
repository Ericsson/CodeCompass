#include <boost/process.hpp>
#include <boost/filesystem.hpp>
namespace bp = boost::process;
namespace fs = boost::filesystem;

#include <string>
#include <iostream>

// compile and run:
// cd ~/Desktop/Codecompass_git/CodeCompass/plugins/csharp/test/src
// g++ csharpparserboosttest.cpp -L /usr/lib/ -lboost_system -lboost_thread -lpthread -lboost_filesystem; ./a.out


int main() {
    std::cout << "\n Testing..." << std::endl;

    fs::path curr_path(fs::current_path());
    std::cout << "Current path is : " << curr_path << std::endl;

    fs::path csharp_path = fs::system_complete("../../parser/src_csharp");
    std::cout << "Csharp path is : " << csharp_path << std::endl;

    std::future<std::string> log;

    int result = bp::system("dotnet run --no-build", bp::start_dir(csharp_path), bp::std_out > log);

    std::cout << "dotnet run return value: " << result << std::endl;
    std::cout << "dotnet run log: " << log.get() << std::endl;

    return 0;
}