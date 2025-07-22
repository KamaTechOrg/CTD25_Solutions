// Graphics.h
#pragma once
#include "Command.h"
#include "Img.h"
#include <vector>

class Graphics {
public:
  Graphics() = default;
  explicit Graphics(std::vector<Img> frames, bool loop = true,
                    double fps = 6.0);

  void reset(const Command &cmd);
  void update(long long nowMs);
  Img &getImg();
  Graphics clone() const { return *this; }

private:
  std::vector<Img> frames_;
  bool loop_{true};
  double fps_{6.0};
  long long startMs_{0};
  size_t curFrame_{0};
};

