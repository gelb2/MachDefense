#include "SessionTestSupport.hpp"
#include "Level10HUDSweep.hpp"
int main(int argc,char** argv){try{if(argc!=2)throw std::runtime_error("Pass Resources directory");Resources resources;resources.root=argv[1];std::cout<<md::validation::runLevel10HUDSweep(resources)<<'\n';}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
