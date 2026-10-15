//
// Created by oleub on 02.01.26.
//

#pragma once

#include "field.hpp"

namespace SAA {
  using namespace sc::literals;

  class Struct : public Field {
  public:
    Struct(std::string_view classname = "Struct", uint64_t clsHash = StaticHASH) : Field(classname, clsHash) {};

    AssetFileParser *m_owner = nullptr;
    int32_t m_superStructIndex{};

    static constexpr  uint64_t StaticHASH = "Struct"_h;


  protected:


    void parse() override;
  };
} 
