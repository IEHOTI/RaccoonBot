#include "Ocr.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include "ImageLibrary/ImageLibrary.h"
#include "CustomError/Exception.h"

#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QImage>

namespace
{
    const cv::Size gTemplateSymbolSize = { 16,16 }; // mean: 16, 21  20,30
}

Ocr::Ocr(QObject *parent): QObject(parent) {}

Ocr::~Ocr() = default;

void Ocr::Initialize() {
    Train();
    mIsLoaded = true;
}
void Ocr::emitError(const QString &str) const {
    const_cast<Ocr*>(this)->sendError(str);
}
void Ocr::Train() {
    std::array<std::vector<cv::Mat>, 10> glyphs;
    ImageLibrary* lib;
    try {
        lib = new ImageLibrary("ImageLib.dll");
        bool result = false;
        lib->load(result);
        if(!result) throw ImageException("Cannot load images");
    } catch(ImageException &e){
        emitError(e.what());
        return;
    }
    // Перебираем цифры 0–9
    for (int d = 0; d <= 9; ++d) {
        QString resourcePath = QString(":/numbers/%1.png").arg(d);
        cv::Mat img;
        lib->get(resourcePath,img);
        if (img.empty()) {
            emitError("Не удалось загрузить " + resourcePath);
            return;
        }
        // Конвертируем в grayscale если нужно
        cv::Mat gray_img;
        cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
        glyphs[d].push_back(gray_img.clone());
    }

    cv::Mat array_chars;
    cv::Mat array_images;

    for (size_t w = 0; w < glyphs.size(); ++w) {
        for (const auto &img : glyphs[w]) {
            cv::Mat template_img;
            resize(img, template_img, gTemplateSymbolSize);

            cv::Mat template_img_float;
            template_img.convertTo(template_img_float, CV_32FC1);
            cv::Mat template_img_reshape = template_img_float.reshape(1, 1);

            array_images.push_back(template_img_reshape);
            array_chars.push_back('0' + static_cast<int>(w));
        }
    }

    std::scoped_lock lock(mKnearesMutex);
    if (!array_images.empty()) {
        mKNearest = cv::ml::KNearest::create();
        mKNearest->setDefaultK(1);
        if (!mKNearest->train(array_images, cv::ml::ROW_SAMPLE, array_chars)) emitError("Не удалось обучить модель.");
    }
    delete lib;
    lib = nullptr;
}

int Ocr::RecognizeDigit(const cv::Mat &img) const {
    if (img.empty()) return false;

    cv::Mat template_img;
    resize(img, template_img, gTemplateSymbolSize);
    cv::Mat template_img_float;
    template_img.convertTo(template_img_float, CV_32FC1);
    cv::Mat template_img_reshape = template_img_float.reshape(1, 1);

    float res = -1;
    {
        std::scoped_lock lock(mKnearesMutex);
        if (!mKNearest){
            emitError("Отсутствует модель.");
            return -2;
        }
        try {
            cv::Mat finded_symbol_img(0, 0, CV_32F);
            res = mKNearest->findNearest(template_img_reshape, 1, finded_symbol_img);
        }
        catch (const std::exception &e) {
            emitError(e.what());
            return -3;
        }
    }

    auto symbol = static_cast<char>(res);
    if (symbol >= '0'  &&symbol <= '9') return static_cast<int>(symbol - '0');

    emitError("Цифра не распознана");
    return -1;
}
void Ocr::Recognize(const cv::Mat &img,int &num) const {
    if (img.empty()) {
        num = -1;
        emitError("Пустое изображение на входе");
        return;
    }
    //imwrite(("C:/Utilities/Coding/Photo/ocr/preprocessed_before.png"), img);
    auto preprocessed = Preprocess(img);
    //imwrite(("C:/Utilities/Coding/Photo/ocr/preprocessed_after.png"), img);
    auto glyphs = FindGlyphs(preprocessed);

    if (glyphs.empty()) {
        num = -1;
        emitError("Глифы цифр не найдены.");
        return;
    }

    int number = 0;
    for (size_t i = 0; i < glyphs.size(); ++i) {
        auto digit = RecognizeDigit(cv::Mat(preprocessed, glyphs[glyphs.size() - i - 1]));

        if (digit < 0) {
            num = digit;
            emitError("Цифра не распознана");
            return;
        } else if (digit > 9) {
            num = digit;
            emitError("Цифра не распознана");
            return;
        }

        int power = static_cast<int>(std::pow(10, i));
        number += digit * power;
    }
    num = number;
}

bool Ocr::IsLoaded() const {
    return mIsLoaded;
}

cv::Mat Ocr::DrawRects(const cv::Mat &img, const std::vector<cv::Rect> &rects) const {
    cv::Mat out;
    img.copyTo(out);
    for (const auto &rect : rects) rectangle(out, rect, cv::Scalar(255, 0, 0), cv::FILLED);

    return out;
}

cv::Mat Ocr::Preprocess(const cv::Mat &img) const {
    cv::Mat gray;
    cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::Mat out;
    threshold(gray, out, 230, 255, cv::THRESH_BINARY);

    return out;
}

std::vector<cv::Rect> Ocr::FindGlyphs(const cv::Mat &img) const {
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> out;
    for (const auto &contour : contours) {
        auto brect = boundingRect(contour);

        if (brect.size().width < mParams.glyph_min_width) continue;

        if (brect.size().height < mParams.glyph_min_height) continue;

        if (brect.area() < mParams.glyph_min_area) continue;

        if (brect.size().width > mParams.glyph_max_width) continue;

        if (brect.size().height > mParams.glyph_max_height) continue;

        if (brect.area() > mParams.glyph_max_area) continue;

        out.push_back(brect);
    }
    //Mat temp = DrawRects(img,out);
    //imshow("001",temp);

    sort(out.begin(), out.end(), [](const cv::Rect &a, const cv::Rect &b) {
             return (a.br() + a.tl()).x / 2 < (b.br() + b.tl()).x / 2;
         });
    return out;
}
