#pragma once
#include <dlms/core/types.hpp>
#include <dlms/core/cosem_object.hpp>
#include <dlms/core/obis.hpp>
#include <dlms/core/dlms_variant.hpp>

#include <vector>
#include <string>

namespace dlms::cosem {

inline DlmsValue make_dlms_array(const std::vector<DlmsValue>& items);
inline DlmsValue make_dlms_structure(const std::vector<DlmsValue>& elements);

/**
 * IC18 Image Transfer
 * 
 * Attributes:
 * 1: logical_name (octet-string)
 * 2: image_identifier (octet-string)
 * 3: image_size (double-long-unsigned)
 * 4: image_transfer_status (enum)
 * 5: image_to_activate_info (array of structures)
 * 
 * Methods:
 * 1: image_transfer_initiate
 * 2: image_block_transfer
 * 3: image_verify
 * 4: image_activate
 */
enum class ImageTransferStatus : uint8_t {
    NotInitiated = 0,
    Initiated = 1,
    VerificationSuccessful = 2,
    VerificationFailed = 3,
    WaitingForActivation = 4,
    ActivationSuccessful = 5,
    ActivationFailed = 6,
};

class ImageTransfer : public CosemObject {
public:
    uint16_t class_id() const override { return static_cast<uint16_t>(CosemInterface::ImageTransfer); }
    uint8_t version() const override { return 0; }
    std::string class_name() const override { return "ImageTransfer"; }

    std::vector<uint8_t> image_identifier_;
    uint32_t image_size_ = 0;
    ImageTransferStatus status_ = ImageTransferStatus::NotInitiated;
    std::vector<uint8_t> image_data_;
    uint32_t transferred_blocks_ = 0;

    std::vector<AttributeAccess> get_attributes() const override {
        return {
            AttributeAccess{1, "logical_name", true},
            AttributeAccess{2, "image_identifier", false},
            AttributeAccess{3, "image_size", false},
            AttributeAccess{4, "image_transfer_status", false},
            AttributeAccess{5, "image_to_activate_info", false}
        };
    }

    Result<DlmsValue> get_attribute(int id) const override {
        if (id == 1) { 
            auto a = ln_.to_array(); 
            return DlmsValue(std::vector<uint8_t>(a.begin(), a.end())); 
        }
        if (id == 2) return DlmsValue(image_identifier_);
        if (id == 3) return DlmsValue(static_cast<uint32_t>(image_size_));
        if (id == 4) return DlmsValue(static_cast<uint8_t>(status_));
        if (id == 5) {
            // Image to activate info - simplified
            return make_dlms_array(std::vector<DlmsValue>{});
        }

        return make_err<DlmsValue>(DlmsError::ObjectUndefined);
    }

    VoidResult set_attribute(int id, const DlmsValue& v) override {
        if (id == 2) { if (auto* p = std::get_if<std::vector<uint8_t>>(&v)) image_identifier_ = *p; return make_void_ok(); }
        if (id == 3) { if (auto* p = std::get_if<uint32_t>(&v)) image_size_ = *p; return make_void_ok(); }

        return make_err<std::monostate>(DlmsError::AccessDenied);
    }

    VoidResult initiate(const std::vector<uint8_t>& identifier, uint32_t size) {
        image_identifier_ = identifier;
        image_size_ = size;
        image_data_.clear();
        image_data_.reserve(size);
        transferred_blocks_ = 0;
        status_ = ImageTransferStatus::Initiated;
        return make_void_ok();
    }

    VoidResult transfer_block(uint32_t block_index, const std::vector<uint8_t>& block_data) {
        if (status_ != ImageTransferStatus::Initiated) {
            return make_err<std::monostate>(DlmsError::NotSupported);
        }
        image_data_.insert(image_data_.end(), block_data.begin(), block_data.end());
        transferred_blocks_++;
        return make_void_ok();
    }

    VoidResult verify() {
        if (image_data_.size() >= image_size_) {
            status_ = ImageTransferStatus::VerificationSuccessful;
        } else {
            status_ = ImageTransferStatus::VerificationFailed;
        }
        return make_void_ok();
    }

    VoidResult activate() {
        if (status_ == ImageTransferStatus::VerificationSuccessful) {
            status_ = ImageTransferStatus::ActivationSuccessful;
        } else {
            status_ = ImageTransferStatus::ActivationFailed;
        }
        return make_void_ok();
    }

    ImageTransferStatus status() const { return status_; }
};

} // namespace dlms::cosem
