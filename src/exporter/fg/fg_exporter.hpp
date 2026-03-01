//
// Created by oleub on 24.02.26.
//

#pragma once
struct Recipe;

namespace SAP {
  class FGRecipe;

  class FGExporter {
    public:
    static Recipe* encode(FGRecipe *recipe);
  };
} // SAP