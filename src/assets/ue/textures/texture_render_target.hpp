//
// Created by oleub on 16.01.26.
//

#pragma once
#include "texture.hpp"
namespace SAP{
class TextureRenderTarget: public SAP::Texture {
  public:
  TextureRenderTarget(std::string classname = "TextureRenderTarget") : Texture(classname) {}
};}
