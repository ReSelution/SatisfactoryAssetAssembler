//
// Created by oleub on 12.12.25.
//

#pragma once
#include "texture.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class Texture2D : public Texture {

  public:
    Texture2D();
    ~Texture2D() override = default;

    void parse(size_t &cursor, AssetFileParser *owner) override;

  protected:
    IntPoint m_importedSize{};

  };

  SAP_REGISTER_ASSET(Texture2D)
}

