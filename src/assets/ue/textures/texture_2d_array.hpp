//
// Created by oleub on 16.01.26.
//

#pragma once
#include "texture.hpp"

#include "../../object_factory.hpp"

namespace SAA {
  class Texture2DArray : public SAA::Texture {
    friend class TextureExporter;

  public:
    Texture2DArray(std::string_view classname = "Texture2DArray") : Texture(classname) {};
    void decodeTexture(ExportedTextureData &data) override;

  protected:
    void parse() override;
  };
  SAA_REGISTER_ASSET(Texture2DArray)
} 
