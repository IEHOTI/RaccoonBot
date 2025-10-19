#ifndef IMAGELIB_H
#define IMAGELIB_H

#include <QtCore/qglobal.h>

#if defined(IMAGELIB_LIBRARY)
#define IMAGE_API Q_DECL_EXPORT
#else
#define IMAGE_API Q_DECL_IMPORT
#endif

#include "qcontainerfwd.h"
namespace cv { class Mat; }

extern "C" {
// Типы указателей на функции
typedef void (*loadImagesFunc)(bool &result);
typedef void (*getImageFunc)(const QString &path, cv::Mat &image);
typedef void (*hasImageFunc)(const QString &path, bool &result);
typedef void (*updateImageFunc)(const QString &path, const cv::Mat *image);
typedef void (*getListImagesFunc)(QStringList &result);
typedef void (*getVersionFunc)(QString &result);
typedef void (*cleanupFunc)();

// Экспортируемые функции
void IMAGE_API loadImages(bool &result);
void IMAGE_API getImage(const QString &path, cv::Mat &image);
void IMAGE_API hasImage(const QString &path, bool &result);
void IMAGE_API updateImage(const QString &path, const cv::Mat *image);
void IMAGE_API getListImages(QStringList &result);
void IMAGE_API getVersion(QString &result);
void IMAGE_API cleanup();
}

struct Cleaner {
    ~Cleaner();
};

#endif // IMAGELIB_H
