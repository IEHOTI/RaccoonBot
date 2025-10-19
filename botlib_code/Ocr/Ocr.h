#ifndef OCR_H
#define OCR_H

#include "BotLib_global.h"
#include <opencv2/opencv.hpp>
#include <QObject>
#include <mutex>

class BOTLIB_EXPORT Ocr : public QObject {
    Q_OBJECT
public:
    explicit Ocr(QObject* parent = nullptr);
    ~Ocr(); /* = default;*/

    struct Params {
        int gray_threshold = 242;
        int glyph_min_width = 6;//7
        int glyph_min_height = 10;//13
        int glyph_min_area = 59;//90
        int glyph_max_width = 16;//7
        int glyph_max_height = 16;//13
        int glyph_max_area = 256;//90
    };

    Ocr(const Ocr &other) = delete;
    Ocr(Ocr &&other) noexcept = delete;
    Ocr &operator=(const Ocr &other) = delete;
    Ocr &operator=(Ocr &&other) noexcept = delete;

    bool IsLoaded() const;

    int RecognizeDigit(const cv::Mat &img) const;

    cv::Mat Preprocess(const cv::Mat &img) const;
    cv::Mat DrawRects(const cv::Mat &img, const std::vector<cv::Rect> &rects) const;

    std::vector<cv::Rect> FindGlyphs(const cv::Mat &img) const;

public slots:
    void Recognize(const cv::Mat &img, int &num) const; //к нему сигнал void **(const Mat &img, int &num);
    void Initialize();

signals:
    void sendError(const QString &str);
private:
    void Train();
    void emitError(const QString &str) const;

    mutable std::mutex mKnearesMutex;
    cv::Ptr<cv::ml::KNearest> mKNearest;
    QString mTrainDir;

    std::atomic<bool> mIsLoaded = false;
    Params mParams;
};

#endif // OCR_H
