#include "SessionTestSupport.hpp"
#include "EmergencySupportSweep.hpp"
int main(int argc,char** argv){assert(argc==2);Resources r;r.root=argv[1];std::cout<<md::validation::runEmergencySupportSweep(r,[&](const Frame& f,std::string_view){checkImages(f,r);})<<'\n';}
