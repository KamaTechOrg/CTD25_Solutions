#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>

class Img {
public:
  Img() = default;

  // ─────────────── Load ───────────────
  Img &read(const std::string &path, std::pair<int, int> size = {},
            bool keepAspect = false, int interpolation = cv::INTER_AREA);

  // ─────────────── Basic ops ──────────
  void drawOn(Img &other, int x, int y);
  void putText(const std::string &txt, int x, int y, double fontSize,
               cv::Scalar colour = {255, 255, 255, 255}, int thickness = 1);

  void show() const;

  // ─────────────── Accessors ──────────
  const cv::Mat &mat() const { return img_; }
  cv::Mat &mat() { return img_; }

  [[nodiscard]] bool loaded() const { return !img_.empty(); }

  // ─────────────── Utility ────────────
  Img clone() const; 

private:
  cv::Mat img_;
};
