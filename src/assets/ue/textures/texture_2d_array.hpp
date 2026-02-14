//
// Created by oleub on 16.01.26.
//

#pragma once
#include "texture.hpp"

#include "ue/object_factory.hpp"

namespace SAP {
  class Texture2DArray : public SAP::Texture {
    friend class TextureExporter;

  public:
    Texture2DArray(std::string_view classname = "Texture2DArray") : Texture(classname) {};
    void decodeTexture(ExportedTextureData &data) override;

  protected:
    void parse() override;
  };
  SAP_REGISTER_ASSET(Texture2DArray)
} // namespace SAP
