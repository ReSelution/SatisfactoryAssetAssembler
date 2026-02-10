//
// Created by oleub on 12.12.25.
//

#pragma once
#include "../../../util/byte_cursor.hpp"
#include "texture.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class Texture2D : public Texture {

  public:
    Texture2D(std::string_view className = "Texture2D");
    ~Texture2D() override = default;

  protected:
    void parse() override;
    IntPoint m_importedSize{};

  };

  SAP_REGISTER_ASSET(Texture2D)
}

