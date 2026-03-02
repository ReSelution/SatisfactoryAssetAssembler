//
// Created by oleub on 24.02.26.
//

#pragma once

struct ItemDescriptor;
struct Recipe;

namespace SAA {
class FGItemDescriptor;
class FGRecipe;

  class FGExporter {
    public:
    static Recipe* encode(FGRecipe *recipe);
    static ItemDescriptor* encode(FGItemDescriptor *recipe);

  };
} // SAP