//
// Created by oleub on 08.03.26.
//

#pragma once

namespace SAA {
  class FGDescriptorBase;
  class FGBuildingBase;
  class Object;
  class FGItemBase;

  class FGFactory {
    friend class FGExporter;
    static FGItemBase* createItem(Object * obj);
    static FGBuildingBase * createBuilding(Object * obj);
    static FGDescriptorBase * createBase(Object * obj);
  };
} // SAA