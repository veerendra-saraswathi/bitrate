#include "dct.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <cstdint>

namespace {
constexpr int BS = 8;
constexpr int TILE = 64;   // 64x64 = 64 blocks

uint32_t crc32(const uint8_t* data, size_t len) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        c ^= data[i];
        for (int b = 0; b < 8; ++b) c = (c >> 1) ^ (0xEDB88320u & -(c & 1u));
    }
    return ~c;
}

void fdct_1d(const double* in, double* out) {
    for (int k = 0; k < 8; ++k) {
        double s = 0; double ck = (k==0)?1/std::sqrt(2.0):1;
        for (int n = 0; n < 8; ++n) s += in[n]*std::cos((2*n+1)*k*M_PI/16);
        out[k] = ck * s;
    }
}
void idct_1d(const double* in, double* out) {
    for (int n = 0; n < 8; ++n) {
        double s = 0;
        for (int k = 0; k < 8; ++k) {
            double ck = (k==0)?1/std::sqrt(2.0):1;
            s += ck*in[k]*std::cos((2*n+1)*k*M_PI/16);
        }
        out[n] = 0.5 * s;
    }
}
void fdct_8x8(const double* in, double* out) {
    double t[64];
    for (int y=0;y<8;++y) fdct_1d(in+y*8, t+y*8);
    for (int x=0;x<8;++x) {
        double c[8], o[8];
        for (int y=0;y<8;++y) c[y]=t[y*8+x];
        fdct_1d(c, o);
        for (int y=0;y<8;++y) out[y*8+x]=0.25*o[y];
    }
}
void idct_8x8(const double* in, double* out) {
    double t[64];
    for (int x=0;x<8;++x) {
        double c[8], o[8];
        for (int y=0;y<8;++y) c[y]=in[y*8+x];
        idct_1d(c, o);
        for (int y=0;y<8;++y) t[y*8+x]=o[y];
    }
    for (int y=0;y<8;++y) idct_1d(t+y*8, out+y*8);
}

constexpr int ZZ[64]={0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63};

uint32_t zz_enc(int32_t v){return v>=0?(uint32_t)v*2u:(uint32_t)(-(int64_t)v)*2u-1u;}
int32_t zz_dec(uint32_t v){return (v&1)?-(int32_t)((v>>1)+1):(int32_t)(v>>1);}
void wv(uint32_t v, std::vector<uint8_t>& o){while(v>=0x80){o.push_back((v&0x7f)|0x80);v>>=7;}o.push_back(v);}
bool rv(const uint8_t* d,size_t n,size_t& p,uint32_t& v){v=0;for(unsigned s=0;s<=28;s+=7){if(p>=n)return false;uint8_t b=d[p++];v|=(uint32_t)(b&0x7f)<<s;if(!(b&0x80))return true;}return false;}
int dzq(double c,int qp){if(std::abs(c)<qp*0.75)return 0;return (int)std::round(c/qp);}

void enc_blk(const double* coeff, int qs, std::vector<uint8_t>& blk) {
    int32_t q[64]; int last=-1;
    for(int i=0;i<64;++i){int qq=dzq(coeff[ZZ[i]],qs);q[i]=std::clamp(qq,-32768,32767);if(q[i])last=i;}
    int run=0;
    for(int i=0;i<=last;++i){
        if(q[i]==0)++run;
        else{wv(run,blk);wv(zz_enc(q[i]),blk);run=0;}
    }
    int rem=63-last; if(rem>0) wv(rem,blk);
}
void dec_blk(const uint8_t* d, size_t len, int qs, double* coeff) {
    int32_t q[64]={0}; size_t p=0; int i=0;
    while(i<64 && p<len){
        uint32_t run=0; if(!rv(d,len,p,run))break;
        i+=(int)run; if(i>=64)break;
        uint32_t e=0; if(!rv(d,len,p,e))break;
        if(i<64) q[i]=zz_dec(e); ++i;
    }
    for(int k=0;k<64;++k) coeff[ZZ[k]]=(double)q[k]*qs;
}
} // ns

std::vector<uint8_t> dct_encode_frame(const std::vector<uint8_t>& px, int w, int h, int qp) {
    if(w%TILE||h%TILE) throw std::invalid_argument("need multiple of 64");
    std::vector<uint8_t> pay; int qs=std::max(qp,1);
    for(int ty=0;ty<h;ty+=TILE) for(int tx=0;tx<w;tx+=TILE){
        std::vector<uint8_t> tile;
        for(int by=0;by<TILE;by+=BS) for(int bx=0;bx<TILE;bx+=BS){
            double b[64],c[64];
            for(int y=0;y<BS;++y) for(int x=0;x<BS;++x)
                b[y*BS+x]=px[(ty+by+y)*w+(tx+bx+x)]-128.0;
            fdct_8x8(b,c);
            std::vector<uint8_t> blk; enc_blk(c,qs,blk);
            uint16_t bl=(uint16_t)blk.size();
            tile.push_back(bl&0xff); tile.push_back(bl>>8);
            tile.insert(tile.end(), blk.begin(), blk.end());
        }
        uint32_t crc=crc32(tile.data(),tile.size());
        uint32_t len=(uint32_t)tile.size();
        pay.push_back(len&0xff); pay.push_back(len>>8); pay.push_back(len>>16); pay.push_back(len>>24);
        pay.insert(pay.end(), tile.begin(), tile.end());
        pay.push_back(crc&0xff); pay.push_back(crc>>8); pay.push_back(crc>>16); pay.push_back(crc>>24);
    }
    return pay;
}

std::vector<uint8_t> dct_decode_frame(const std::vector<uint8_t>& pay, int w, int h, int qp) {
    std::vector<uint8_t> out(w*h,0); size_t pos=0; int qs=std::max(qp,1);
    for(int ty=0;ty<h && pos+8<=pay.size(); ty+=TILE)
     for(int tx=0;tx<w && pos+8<=pay.size(); tx+=TILE){
        uint32_t len=pay[pos]|(pay[pos+1]<<8)|(pay[pos+2]<<16)|(pay[pos+3]<<24); pos+=4;
        if(pos+len+4>pay.size()) return out;
        const uint8_t* td=pay.data()+pos;
        uint32_t sc=pay[pos+len]|(pay[pos+len+1]<<8)|(pay[pos+len+2]<<16)|(pay[pos+len+3]<<24);
        uint32_t cc=crc32(td,len); pos+=len+4;
        if(cc!=sc) continue;
        size_t tp=0;
        for(int by=0;by<TILE;by+=BS) for(int bx=0;bx<TILE;bx+=BS){
            if(tp+2>len) break;
            uint16_t bl=td[tp]|(td[tp+1]<<8); tp+=2;
            if(tp+bl>len) break;
            double coeff[64]={0};
            dec_blk(td+tp, bl, qs, coeff); tp+=bl;
            double block[64]; idct_8x8(coeff, block);
            for(int y=0;y<BS;++y) for(int x=0;x<BS;++x){
                int v=(int)std::round(block[y*BS+x]+128.0);
                out[(ty+by+y)*w+(tx+bx+x)]=(uint8_t)std::clamp(v,0,255);
            }
        }
     }
    return out;
}
