#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace SAA {
#pragma pack(push, 1)
  struct FVector2f_Layout { float x, y; };
  struct FVector3f_Layout { float x, y, z; };
  struct FVector4f_Layout { float x, y, z, w; };
  struct FVector2d_Layout { double x, y; };
  struct FVector3d_Layout { double x, y, z; };
  struct FVector4d_Layout { double x, y, z, w; };
  struct FIntVector_Layout { int32_t x, y, z; };
  struct FIntVector2_Layout { int32_t x, y; };
#pragma pack(pop)

  // --- Erweiterte Structs mit Logik ---
  template<typename T, typename Layout>
    struct TVector : public Layout {
    // KEIN Konstruktor hier! Konstruktoren machen den Typ non-trivial.

    // Wir nutzen statische Helper-Methoden für die Konvertierung
    static TVector From(const T& v) {
      TVector res;
      res.x = static_cast<decltype(res.x)>(v.x);
      res.y = static_cast<decltype(res.y)>(v.y);
      if constexpr (requires { res.z; v.z; }) res.z = static_cast<decltype(res.z)>(v.z);
      if constexpr (requires { res.w; v.w; }) res.w = static_cast<decltype(res.w)>(v.w);
      return res;
    }

    // Konvertierung zu GLM (bleibt erlaubt und bricht Trivialität nicht)
    operator T() const {
      if constexpr (sizeof(Layout) == 8) return { (float)this->x, (float)this->y };
      else if constexpr (sizeof(Layout) == 12 || sizeof(Layout) == 24) return { (double)this->x, (double)this->y, (double)this->z };
      else return { (double)this->x, (double)this->y, (double)this->z, (double)this->w };
    }
  };

  // --- Aliase (Die "Dinger", die du eigentlich benutzen willst) ---
  using FVector2F = TVector<glm::vec2, FVector2f_Layout>;
  using FVector3F = TVector<glm::vec3, FVector3f_Layout>;
  using FVector4F = TVector<glm::vec4, FVector4f_Layout>;

  using FVector2D = TVector<glm::dvec2, FVector2d_Layout>;
  using FVector = TVector<glm::dvec3, FVector3d_Layout>; // Standard UE5 Vector (Double)
  using FVector4D = TVector<glm::dvec4, FVector4d_Layout>;

  using FIntVector = TVector<glm::ivec3, FIntVector_Layout>;
  using FIntVector2 = TVector<glm::ivec2, FIntVector2_Layout>;

  struct FRotator  {
    double pitch, yaw, roll;
    operator glm::dvec3() const { return {pitch, yaw, roll}; }
  };
}
