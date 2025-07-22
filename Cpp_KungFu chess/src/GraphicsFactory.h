#pragma once
#include "Graphics.h"
#include <filesystem>

class GraphicsFactory {
public:
  /** Build a Graphics object from a sprite folder + JSON‑like cfg.
      @param spritesDir   path to folder containing PNGs
      @param cfg          parsed JSON object (nlohmann::json or std::map)
      @param cellSize     {width, height} in pixels               */
  template <typename Json>
  Graphics load(const std::filesystem::path &spritesDir, const Json &cfg,
                std::pair<int, int> cellSize) const {
    bool loop = cfg.contains("is_loop") ? cfg["is_loop"].get<bool>() : true;
    double fps = cfg.contains("frames_per_sec")
                     ? cfg["frames_per_sec"].get<double>()
                     : 6.0;

    return Graphics(/*sprites_folder*/ spritesDir,
                    /*cell_size     */ cellSize,
                    /*loop          */ loop,
                    /*fps           */ fps);
  }
};
