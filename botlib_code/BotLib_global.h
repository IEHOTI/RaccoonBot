#ifndef BOTLIB_GLOBAL_H
#define BOTLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BOTLIB_LIBRARY)
#define BOTLIB_EXPORT Q_DECL_EXPORT
#else
#define BOTLIB_EXPORT Q_DECL_IMPORT
#endif

#endif // BOTLIB_GLOBAL_H
