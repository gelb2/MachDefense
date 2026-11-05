#include "SessionTestSupport.hpp"
#include "ContentSweep.hpp"
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Pass Resources directory");
    Resources resources;resources.root=argv[1];Host host;
    auto result=md::validation::runContentSweep(resources,host,[&](const Frame& frame){checkImages(frame,resources);});
    std::cout<<"Content: "<<result.builds<<" production checks, "<<result.placements<<" carrier deployments, "<<result.attacks<<" special activations; no file writes.\n";
}catch(const std::exception& e){std::cerr<<"CoreContent failure: "<<e.what()<<'\n';return 1;}}
