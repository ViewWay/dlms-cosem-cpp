#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>
#include <string>
#include <vector>

namespace dlms {

struct AttributeAccess { int attribute_id; std::string name; bool is_static; };

class CosemObject {
public:
    virtual ~CosemObject()=default;
    virtual uint16_t class_id()const=0;
    virtual uint8_t version()const{return 0;}
    virtual std::string class_name()const=0;
    const ObisCode& logical_name()const{return ln_;}
    void set_logical_name(const ObisCode& n){ln_=n;}

    virtual std::vector<AttributeAccess> get_attributes()const{
        return {{1,"logical_name",true}};
    }
    virtual Result<DlmsValue> get_attribute(int id)const{
        if(id==1){auto a=ln_.to_array();return DlmsValue(std::vector<uint8_t>(a.begin(),a.end()));}
        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }
    virtual VoidResult set_attribute(int,const DlmsValue&){return make_err<std::monostate>(DlmsError::AccessDenied);}
    virtual bool is_static_attribute(int id)const{
        for(auto& a:get_attributes())if(a.attribute_id==id)return a.is_static;
        return false;
    }
protected:
    ObisCode ln_;
};

} // namespace dlms
