#ifndef OCR_H
#define OCR_H
#include "Error.h"

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

    int RecognizeDigit(const Mat &img) const;

    Mat Preprocess(const Mat &img) const;
    Mat DrawRects(const Mat &img, const vector<Rect> &rects) const;

    vector<Rect> FindGlyphs(const Mat &img) const;

public slots:
    void Recognize(const Mat &img, int &num) const; //к нему сигнал void **(const Mat &img, int &num);
    void Initialize();

signals:
    void sendError(const QString &str);
private:
    void Train();
    void emitError(const QString &str) const;

    mutable mutex mKnearesMutex;
    Ptr<ml::KNearest> mKNearest;
    QString mTrainDir;

    atomic<bool> mIsLoaded = false;
    Params mParams;
};

#endif // OCR_H
