//
// Created by oleub on 02.01.26.
//

#pragma once
#include <utility>

#include "field.hpp"

namespace SAP {
  class Struct : public Field {
  public:
    Struct(std::string_view classname = "Struct") : Field(classname) {};

    AssetFileParser *m_owner = nullptr;
    int32_t m_superStructIndex{};

  protected:
    void parse() override;
  };
} // namespace SAP
