//
// Created by oleub on 16.01.26.
//

#pragma once
#include "texture_render_target.hpp"

#include "ue/object_factory.hpp"

namespace SAP {
  class TextureRenderTarget2D: public TextureRenderTarget {
    public:
    TextureRenderTarget2D(std::string classname = "TextureRenderTarget2D") : TextureRenderTarget(classname) {}
  };

  SAP_REGISTER_ASSET(TextureRenderTarget2D)
}
