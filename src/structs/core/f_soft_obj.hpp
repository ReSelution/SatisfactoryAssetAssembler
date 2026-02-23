#pragma once
#include "name.hpp"
//
// Created by oleub on 19.02.26.
//
namespace  SAP{
struct SoftObj {
  FName pkgName;
  FName assetName;
  std::span<const uint8_t> subPath;
};}