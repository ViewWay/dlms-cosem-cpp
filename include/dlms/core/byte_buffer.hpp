#pragma once
#include <dlms/core/types.hpp>
#include <vector>
#include <cstdint>
#include <string>

namespace dlms {

class ByteBuffer {
public:
    ByteBuffer() = default;
    explicit ByteBuffer(size_t cap) { data_.reserve(cap); }
    ByteBuffer(const uint8_t* d, size_t n) : data_(d, d+n) {}

    Result<uint8_t> read_u8() {
        if (pos_ >= data_.size()) return make_err<uint8_t>(DlmsError::BufferUnderflow);
        return data_[pos_++];
    }
    Result<int8_t> read_i8() { auto r=read_u8(); if(!is_ok(r)) return make_err<int8_t>(get_err(r)); return static_cast<int8_t>(get_val(r)); }
    Result<uint16_t> read_u16() {
        if (pos_+2>data_.size()) return make_err<uint16_t>(DlmsError::BufferUnderflow);
        uint16_t v=(uint16_t(data_[pos_])<<8)|data_[pos_+1]; pos_+=2; return v;
    }
    Result<int16_t> read_i16() { auto r=read_u16(); if(!is_ok(r)) return make_err<int16_t>(get_err(r)); return static_cast<int16_t>(get_val(r)); }
    Result<uint32_t> read_u32() {
        if (pos_+4>data_.size()) return make_err<uint32_t>(DlmsError::BufferUnderflow);
        uint32_t v=(uint32_t(data_[pos_])<<24)|(uint32_t(data_[pos_+1])<<16)|(uint32_t(data_[pos_+2])<<8)|data_[pos_+3]; pos_+=4; return v;
    }
    Result<int32_t> read_i32() { auto r=read_u32(); if(!is_ok(r)) return make_err<int32_t>(get_err(r)); return static_cast<int32_t>(get_val(r)); }
    Result<uint64_t> read_u64() {
        if (pos_+8>data_.size()) return make_err<uint64_t>(DlmsError::BufferUnderflow);
        uint64_t v=0; for(int i=0;i<8;++i) v=(v<<8)|data_[pos_+i]; pos_+=8; return v;
    }
    Result<int64_t> read_i64() { auto r=read_u64(); if(!is_ok(r)) return make_err<int64_t>(get_err(r)); return static_cast<int64_t>(get_val(r)); }
    Result<std::vector<uint8_t>> read_bytes(size_t n) {
        if (pos_+n>data_.size()) return make_err<std::vector<uint8_t>>(DlmsError::BufferUnderflow);
        std::vector<uint8_t> v(data_.begin()+pos_, data_.begin()+pos_+n); pos_+=n; return v;
    }
    Result<size_t> read_variable_length() {
        auto first=read_u8(); if(!is_ok(first)) return make_err<size_t>(get_err(first));
        uint8_t f=get_val(first);
        if (!(f&0x80)) return f;
        int nb=f&0x7F; if(nb==0||nb>4) return make_err<size_t>(DlmsError::InvalidLength);
        size_t len=0;
        for(int i=0;i<nb;++i){auto b=read_u8();if(!is_ok(b))return make_err<size_t>(get_err(b));len=(len<<8)|get_val(b);}
        return len;
    }

    void write_u8(uint8_t v){data_.push_back(v);}
    void write_i8(int8_t v){data_.push_back(static_cast<uint8_t>(v));}
    void write_u16(uint16_t v){data_.push_back((v>>8)&0xFF);data_.push_back(v&0xFF);}
    void write_i16(int16_t v){write_u16(static_cast<uint16_t>(v));}
    void write_u32(uint32_t v){data_.push_back((v>>24)&0xFF);data_.push_back((v>>16)&0xFF);data_.push_back((v>>8)&0xFF);data_.push_back(v&0xFF);}
    void write_i32(int32_t v){write_u32(static_cast<uint32_t>(v));}
    void write_u64(uint64_t v){for(int i=7;i>=0;--i)data_.push_back((v>>(i*8))&0xFF);}
    void write_i64(int64_t v){write_u64(static_cast<uint64_t>(v));}
    void write_bytes(const uint8_t* d,size_t n){data_.insert(data_.end(),d,d+n);}
    void write_bytes(const std::vector<uint8_t>& v){write_bytes(v.data(),v.size());}
    void write_string(const std::string& s){write_bytes(reinterpret_cast<const uint8_t*>(s.data()),s.size());}
    void write_variable_length(size_t len){
        if(len<=0x7F){write_u8(static_cast<uint8_t>(len));}
        else{int nb=0;size_t t=len;while(t>0){++nb;t>>=8;}
            write_u8(static_cast<uint8_t>(0x80|nb));
            for(int i=nb-1;i>=0;--i)write_u8(static_cast<uint8_t>((len>>(i*8))&0xFF));}
    }

    size_t position()const{return pos_;}
    void set_position(size_t p){pos_=p;}
    size_t remaining()const{return data_.size()-pos_;}
    size_t size()const{return data_.size();}
    bool empty()const{return pos_>=data_.size();}
    bool has_data()const{return pos_<data_.size();}
    void clear(){data_.clear();pos_=0;}
    const uint8_t* data()const{return data_.data();}
    uint8_t* data(){return data_.data();}
    const std::vector<uint8_t>& vector()const{return data_;}
    std::vector<uint8_t> remaining_bytes()const{return std::vector<uint8_t>(data_.begin()+pos_,data_.end());}
    void reset_read(){pos_=0;}
    uint8_t peek()const{return pos_<data_.size()?data_[pos_]:0;}
private:
    std::vector<uint8_t> data_;
    size_t pos_=0;
};

} // namespace dlms
