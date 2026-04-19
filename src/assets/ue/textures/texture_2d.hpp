//
// Created by oleub on 12.12.25.
//

#pragma once
#include "texture.hpp"
#include "object_factory.hpp"

namespace SAA {
  class Texture2D : public Texture {

  public:
    Texture2D(std::string_view className = "Texture2D");
    ~Texture2D() override = default;

  protected:
    void parse() override;
    IntPoint m_importedSize{};
  };

  SAA_REGISTER_ASSET(Texture2D)
} 
