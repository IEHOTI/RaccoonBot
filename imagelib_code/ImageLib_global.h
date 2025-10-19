#ifndef IMAGELIB_GLOBAL_H
#define IMAGELIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(IMAGELIB_LIBRARY)
#define IMAGELIB_EXPORT Q_DECL_EXPORT
#else
#define IMAGELIB_EXPORT Q_DECL_IMPORT
#endif

#endif // IMAGELIB_GLOBAL_H
