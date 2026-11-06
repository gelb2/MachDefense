#include "Audio.hpp"
#include <filesystem>
#include <stdexcept>
namespace md::web {
void Audio::prepare(Assets& a){
 if(!valid())throw std::runtime_error(error());
 for(auto& p:std::filesystem::directory_iterator("/assets/Audio")){
  if(p.path().extension()!=".wav")continue;auto name=p.path().filename().string();
  if(!OpenALEffects::prepare(name,decodeWav(a.read("Audio/"+name))))throw std::runtime_error("Cannot prepare "+name);
 }
}
}
