#pragma once
#include "Core/Session/SaveData.hpp"
#include <algorithm>
#include <stdexcept>
namespace md::transfer {
inline std::uint32_t crc(std::span<const std::uint8_t> b){std::uint32_t c=~0u;for(auto x:b){c^=x;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&-(c&1));}return ~c;}
inline constexpr char alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
inline std::string encode(const SaveData& save){
 auto b=save.plaintext();std::vector<std::uint8_t> packed{1};auto check=crc(b);for(int i=0;i<4;++i)packed.push_back(check>>(i*8));
 for(size_t p=0;p<b.size();){size_t run=1;while(p+run<b.size()&&run<128&&b[p+run]==b[p])++run;if(run>=3){packed.push_back(128|(run-1));packed.push_back(b[p]);p+=run;}else{size_t start=p;p+=run;while(p<b.size()&&p-start<128){size_t r=1;while(p+r<b.size()&&r<3&&b[p+r]==b[p])++r;if(r==3)break;p+=std::min(r,128-(p-start));}packed.push_back(p-start-1);packed.insert(packed.end(),b.begin()+start,b.begin()+p);}}
 std::string out="MD1-";std::uint32_t bits=0;int n=0;for(auto c:packed){bits=(bits<<8)|c;n+=8;while(n>=6){n-=6;out+=alphabet[(bits>>n)&63];}}if(n)out+=alphabet[(bits<<(6-n))&63];return out;
}
inline SaveData decode(std::string_view code){
 if(code.size()>120000)throw std::invalid_argument("Recovery code is too long.");
 std::string s;for(unsigned char c:code)if(c!=' '&&c!='\n'&&c!='\r'&&c!='\t')s+=c;
 if(!s.starts_with("MD1-"))throw std::invalid_argument("Enter a MachDefense recovery code.");s.erase(0,4);
 if(s.size()%4==1)throw std::invalid_argument("Recovery code is incomplete.");
 std::vector<std::uint8_t> packed;std::uint32_t bits=0;int n=0;for(char c:s){auto it=std::find(alphabet,alphabet+64,c);if(it==alphabet+64)throw std::invalid_argument("Invalid recovery code character.");bits=(bits<<6)|unsigned(it-alphabet);n+=6;if(n>=8){n-=8;packed.push_back(bits>>n);}}
 if(n&&(bits&((1u<<n)-1)))throw std::invalid_argument("Invalid recovery code ending.");
 if(packed.size()<5||packed[0]!=1)throw std::invalid_argument("Unsupported recovery code version.");std::uint32_t expected=0;for(int i=0;i<4;++i)expected|=std::uint32_t(packed[1+i])<<(8*i);
 std::vector<std::uint8_t> plain;plain.reserve(SaveData::byteCount);
 for(size_t p=5;p<packed.size();){unsigned tag=packed[p++],len=(tag&127)+1;if(plain.size()+len>SaveData::byteCount)throw std::invalid_argument("Invalid recovery data length.");if(tag&128){if(p==packed.size())throw std::invalid_argument("Recovery code is incomplete.");plain.insert(plain.end(),len,packed[p++]);}else{if(len>packed.size()-p)throw std::invalid_argument("Recovery code is incomplete.");plain.insert(plain.end(),packed.begin()+p,packed.begin()+p+len);p+=len;}}
 if(plain.size()!=SaveData::byteCount||crc(plain)!=expected)throw std::invalid_argument("Recovery code checksum failed.");
 auto save=SaveData::fromPlaintext(plain);
 // Existing accessors check entry counts and strings. Preserve unknown fields.
 for(size_t i=0;i<5;++i)if(save.active(i)){if(save.stage(i)>80||save.name(i).empty()||save.credits(i)<0)throw std::invalid_argument("Invalid profile values.");(void)save.builds(i);(void)save.attacks(i);}
 return save;
}
}
