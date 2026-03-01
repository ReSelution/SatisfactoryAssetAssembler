//
// Created by oleub on 24.02.26.
//

#pragma once
#include <cstdint>
#include <span>

enum class DataType { Mesh, Texture };

struct gpu_export {
  DataType type;
  explicit gpu_export(DataType t) : type(t) {}
};

struct MeshData : public gpu_export {
  std::span<uint8_t> vertices;
  std::span<uint8_t> indices;
  uint32_t vertexCount;

  MeshData(std::span<uint8_t> v, std::span<uint8_t> i, uint32_t count)
      : gpu_export(DataType::Mesh), vertices(v), indices(i), vertexCount(count) {}
};

struct TextureData : public gpu_export {
  std::span<uint8_t> pixels;
  int width, height;

  TextureData(std::span<uint8_t> p, int w, int h)
      : gpu_export(DataType::Texture), pixels(p), width(w), height(h) {}
};