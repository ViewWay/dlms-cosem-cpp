#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>
#include <memory>

namespace dlms::cosem {

// Helper functions for creating DlmsArray and DlmsStructure
inline DlmsValue make_dlms_array(const std::vector<DlmsValue>& items) {
    DlmsArray arr;
    for (const auto& item : items) {
        arr.items.push_back(std::make_shared<DlmsValue>(item));
    }
    return DlmsValue(std::move(arr));
}

inline DlmsValue make_dlms_structure(const std::vector<DlmsValue>& elements) {
    DlmsStructure s;
    for (const auto& elem : elements) {
        s.elements.push_back(std::make_shared<DlmsValue>(elem));
    }
    return DlmsValue(std::move(s));
}

/**
 * IC15 Association LN (Logical Name) - DLMS Logical Name Association
 * 
 * This class is used to model an association using logical naming (LN).
 * It's fundamental for DLMS/COSEM client-server communication.
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: object_list (array of structures: class_id, logical_name)
 * 3: associated_partners (structure: client_sap, server_sap)
 * 4: application_context_name (octet-string)
 * 5: authentication_mechanism_name (octet-string)
 * 6: lls_secret (octet-string)
 * 7: hls_secret (octet-string)
 * 8: authentication_status (boolean)
 * 9: security_setup_reference (octet-string)
 */
class AssociationLN : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::AssociationLN); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "AssociationLN"; }

    struct ObjectEntry {
        uint16_t class_id;
        ObisCode logical_name;
    };

    std::vector<ObjectEntry> object_list_;
    uint16_t client_sap_ = 16;
    uint16_t server_sap_ = 1;
    std::vector<uint8_t> application_context_name_;
    std::vector<uint8_t> authentication_mechanism_name_;
    std::vector<uint8_t> lls_secret_;
    std::vector<uint8_t> hls_secret_;
    bool authentication_status_ = false;
    std::vector<uint8_t> security_setup_reference_;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "object_list", false},
            AttributeAccess{3, "associated_partners", true},
            AttributeAccess{4, "application_context_name", true},
            AttributeAccess{5, "authentication_mechanism_name", true},
            AttributeAccess{6, "lls_secret", true},
            AttributeAccess{7, "hls_secret", true},
            AttributeAccess{8, "authentication_status", false},
            AttributeAccess{9, "security_setup_reference", true}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) {
            std::vector<DlmsValue> arr;
            for (const auto& obj : object_list_) {
                auto ob = obj.logical_name.to_array();
                arr.push_back(make_dlms_structure({
                    DlmsValue(static_cast<uint16_t>(obj.class_id)),
                    DlmsValue(std::vector<uint8_t>(ob.begin(), ob.end()))
                }));
            }
            return make_dlms_array(arr);
        }
        if (id == 3) {
            return make_dlms_structure({
                DlmsValue(client_sap_),
                DlmsValue(server_sap_)
            });
        }
        if (id == 4) return DlmsValue(application_context_name_);
        if (id == 5) return DlmsValue(authentication_mechanism_name_);
        if (id == 6) return DlmsValue(lls_secret_);
        if (id == 7) return DlmsValue(hls_secret_);
        if (id == 8) return DlmsValue(authentication_status_);
        if (id == 9) return DlmsValue(security_setup_reference_);

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 3) {
            if (auto* st = std::get_if<DlmsStructure>(&v)) {
                if (st->elements.size() >= 2) {
                    auto& elem0 = *st->elements[0];
                    if (auto* c = std::get_if<uint16_t>(&elem0)) client_sap_ = *c;
                    auto& elem1 = *st->elements[1];
                    if (auto* s = std::get_if<uint16_t>(&elem1)) server_sap_ = *s;
                }
            }
            return make_void_ok();
        }
        if (id == 4) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) application_context_name_ = *p; return make_void_ok(); }
        if (id == 5) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) authentication_mechanism_name_ = *p; return make_void_ok(); }
        if (id == 6) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) lls_secret_ = *p; return make_void_ok(); }
        if (id == 7) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) hls_secret_ = *p; return make_void_ok(); }
        if (id == 8) { if (auto* p = std::get_if<bool>(&v)) authentication_status_ = *p; return make_void_ok(); }
        if (id == 9) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) security_setup_reference_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    void add_object(uint16_t class_id, const ObisCode& ln) {
        object_list_.push_back({class_id, ln});
    }
    std::pair<uint16_t, uint16_t> get_partners() const { return {client_sap_, server_sap_}; }
    void set_partners(uint16_t client, uint16_t server) { client_sap_ = client; server_sap_ = server; }
    bool is_authenticated() const { return authentication_status_; }
    void set_authenticated(bool status) { authentication_status_ = status; }
};

} // namespace dlms::cosem
