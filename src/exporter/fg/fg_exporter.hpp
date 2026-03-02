//
// Created by oleub on 24.02.26.
//

#pragma once

namespace SAA {
class FGItemDescriptor;
class FGRecipe;
struct ItemDescriptor;
struct Recipe;

class FGExporter {
public:
  static Recipe *encode(FGRecipe *recipe);
  static ItemDescriptor *encode(FGItemDescriptor *recipe);
};
} // namespace SAA