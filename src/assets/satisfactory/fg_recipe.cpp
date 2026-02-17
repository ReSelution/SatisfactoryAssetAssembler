//
// Created by oleub on 20.01.26.
//
#include "fg_recipe.hpp"

#include "PropertyTracker.hpp"

namespace SAP {

  void FGRecipe::parse() {
    Object::parse();
    DBG_TRACK_MAP(m_className, m_properties);
    DBG_MARK_HANDLED(m_className, Recipe::ManufacturingMenuPriority);
    DBG_MARK_HANDLED(m_className, Recipe::RelevantEvents);
  }
} // namespace SAP
