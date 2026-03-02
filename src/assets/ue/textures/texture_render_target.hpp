//
// Created by oleub on 16.01.26.
//

#pragma once
#include "texture.hpp"
namespace SAA {
  class TextureRenderTarget : public SAA::Texture {
  public:
    TextureRenderTarget(std::string classname = "TextureRenderTarget") : Texture(classname) {}
  };
} // namespace SAP
