//
// Created by oleub on 23.02.26.
//

#pragma once
#include <cstdint>


enum class EClassSerializationControlExtension : uint8_t {
  NoExtension = 0x00,
  ReserveForFutureUse = 0x01, // Can be use to add a next group of extension

  ////////////////////////////////////////////////
  // First extension group
  OverridableSerializationInformation = 0x02,

  //
  // Add more extension for the first group here
  //
};

// Bitwise OR: Kombiniert Flags
inline EClassSerializationControlExtension operator|(EClassSerializationControlExtension lhs, EClassSerializationControlExtension rhs) {
  return static_cast<EClassSerializationControlExtension>(
      static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
  );
}

// Bitwise AND: Prüft auf Überschneidungen
inline EClassSerializationControlExtension operator&(EClassSerializationControlExtension lhs, EClassSerializationControlExtension rhs) {
  return static_cast<EClassSerializationControlExtension>(
      static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
  );
}

// Bitwise XOR: Schaltet Flags um
inline EClassSerializationControlExtension operator^(EClassSerializationControlExtension lhs, EClassSerializationControlExtension rhs) {
  return static_cast<EClassSerializationControlExtension>(
      static_cast<uint8_t>(lhs) ^ static_cast<uint8_t>(rhs)
  );
}

// Bitwise NOT: Invertiert Flags
inline EClassSerializationControlExtension operator~(EClassSerializationControlExtension arg) {
  return static_cast<EClassSerializationControlExtension>(~static_cast<uint8_t>(arg));
}

// Assignment OR: Hinzufügen (lhs |= rhs)
inline EClassSerializationControlExtension& operator|=(EClassSerializationControlExtension& lhs, EClassSerializationControlExtension rhs) {
  lhs = lhs | rhs;
  return lhs;
}

// Assignment AND: Filtern (lhs &= rhs)
inline EClassSerializationControlExtension& operator&=(EClassSerializationControlExtension& lhs, EClassSerializationControlExtension rhs) {
  lhs = lhs & rhs;
  return lhs;
}

// Hilfsfunktion für bequeme Checks (bool-Konvertierung)
inline bool hasFlag(EClassSerializationControlExtension value, EClassSerializationControlExtension flag) {
  return (value & flag) == flag;
}