#include "Img.h"
#include <stdexcept>

Img& Img::read(const std::string& path,
               std::pair<int,int> size,
               bool keepAspect,
               int interpolation)
{
    img_ = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img_.empty())
        throw std::runtime_error("Cannot load image: " + path);

    if (size != std::pair<int,int>{}) {
        int targetW = size.first,  targetH = size.second;
        int h = img_.rows,         w = img_.cols;

        if (keepAspect) {
            double scale = std::min(static_cast<double>(targetW)/w,
                                    static_cast<double>(targetH)/h);
            cv::resize(img_, img_, {int(w*scale), int(h*scale)}, 0, 0, interpolation);
        } else {
            cv::resize(img_, img_, {targetW, targetH}, 0, 0, interpolation);
        }
    }
    return *this;
}

void Img::drawOn(Img &other, int x, int y) {
    if (!other.loaded() || !loaded())
        return;
    cv::Rect roiR(x, y, img_.cols, img_.rows);
    cv::Mat roi = other.mat()(roiR); // target slice

    if (img_.channels() == 4 && roi.channels() == 4) {
        // Copy all 4 channels, using the alpha as mask
        std::vector<cv::Mat> ch;
        cv::split(img_, ch);    
        img_.copyTo(roi, ch[3]); // copy using A as mask
    } else {
        img_.copyTo(roi); 
    }
}

void Img::putText(const std::string& txt, int x, int y, double fontSize,
                  cv::Scalar colour, int thickness)
{
    if (img_.empty()) throw std::logic_error("Image not loaded");
    cv::putText(img_, txt, {x,y}, cv::FONT_HERSHEY_SIMPLEX,
                fontSize, colour, thickness, cv::LINE_AA);
}

void Img::show() const
{
    if (img_.empty()) throw std::logic_error("Image not loaded");
    cv::imshow("Image", img_);
    cv::waitKey(0);
    cv::destroyAllWindows();
}
