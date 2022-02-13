#pragma once

#include "esphome/core/preferences.h"

#include "../tion-api/tion-api-3s.h"
#include "../tion/tion.h"

namespace esphome {
namespace tion {

using namespace dentra::tion;

class Tion3s : public TionComponent, public TionClimate, public TionBleNode, public TionsApi3s {
 public:
  float get_setup_priority() const override {
    // we should setup after esp32_ble_tracker
    return setup_priority::AFTER_BLUETOOTH;
  }
  void setup() override;
  void update() override {
    if (this->pair_state_ > 0) {
      this->parent_->set_enabled(true);
    }
  }

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  const esp_bt_uuid_t &get_ble_service() const override;
  const esp_bt_uuid_t &get_ble_char_tx() const override;
  const esp_bt_uuid_t &get_ble_char_rx() const override;
  esp_ble_sec_act_t get_ble_encryption() const override { return this->ble_sec_enc_; }
  bool ble_reg_for_notify() const override { return this->pair_state_ > 0; }

  void set_ble_encryption(esp_ble_sec_act_t ble_sec_enc) { this->ble_sec_enc_ = ble_sec_enc; }
  void set_direct_write(bool direct_write) { this->direct_write_ = direct_write; }

  void read_data(const uint8_t *data, uint16_t size) override { TionsApi3s::read_data(data, size); }
  bool write_data(const uint8_t *data, uint16_t size) const override { return TionBleNode::write_data(data, size); }

  void on_ready() override;
  void read(const tion3s_state_t &state) override;
  bool write_state() override;

  void pair() {
    this->pair_state_ = -1;
    this->parent_->set_enabled(true);
  }

  void reset_pair() {
    this->pair_state_ = 0;
    this->parent_->set_enabled(false);
  }

 protected:
  ESPPreferenceObject rtc_;
  bool dirty_{};
  int8_t pair_state_{};  // 0 - not paired, 1 - paired, -1 - pairing
  void update_state_(tion3s_state_t &state);
  esp_ble_sec_act_t ble_sec_enc_{/*esp_ble_sec_act_t::ESP_BLE_SEC_ENCRYPT_MITM*/};
  bool direct_write_{};
};

class Tion3sBuzzerSwitch : public switch_::Switch {
 public:
  explicit Tion3sBuzzerSwitch(Tion3s *parent) : parent_(parent) {}
  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->write_state();
  }

 protected:
  Tion3s *parent_;
};

}  // namespace tion
}  // namespace esphome