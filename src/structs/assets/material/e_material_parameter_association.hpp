//
// Created by oleub on 13.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
    enum class EMaterialParameterAssociation : uint64_t {
        LayerParameter = "LayerParameter"_h,
        BlendParameter = "BlendParameter"_h,
        GlobalParameter = "GlobalParameter"_h,
    };
}
