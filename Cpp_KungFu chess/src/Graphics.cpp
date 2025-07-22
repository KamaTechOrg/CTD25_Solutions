// Graphics.cpp
#include "Graphics.h"
Graphics::Graphics(std::vector<Img> f, bool lp, double fps)
    : frames_(std::move(f)), loop_(lp), fps_(fps) {}

void Graphics::reset(const Command &cmd) {
  startMs_ = cmd.timestamp_ms;
  curFrame_ = 0;
}
void Graphics::update(long long now) {
  if (frames_.size() < 2)
    return;
  double per = 1000.0 / fps_;
  size_t idx = size_t((now - startMs_) / per);
  curFrame_ = loop_ ? idx % frames_.size() : std::min(idx, frames_.size() - 1);
}
Img &Graphics::getImg() { return frames_.front(); /* safe for tests */ }
