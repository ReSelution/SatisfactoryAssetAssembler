//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <iomanip>
#include <string>

namespace SAA {
#pragma pack(push, 1)
  struct FBulkDataCookedIndex {
    uint8_t Value;

    [[nodiscard]] bool isDefault() const { return Value == 0; }

    [[nodiscard]] std::string getAsExtension() const {
      if (isDefault())
        return {};
      std::ostringstream oss;
      oss << std::setw(3) << std::setfill('0') << static_cast<int>(Value);
      return oss.str();
    }

    [[nodiscard]] std::string toString() const { return getAsExtension(); }

    static const FBulkDataCookedIndex Default;
  };
#pragma pack(pop)
  static_assert(sizeof(FBulkDataCookedIndex) == 1, "FBulkDataCookedIndex size mismatch!");

} // namespace SAP
