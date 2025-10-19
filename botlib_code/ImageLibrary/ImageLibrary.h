#ifndef IMAGELIBRARY_H
#define IMAGELIBRARY_H
#include "ImageLib.h"
#include "BotLib_global.h"

class QLibrary;

struct BOTLIB_EXPORT ImageLibrary {
    ImageLibrary(const QString &name);
    ~ImageLibrary();

    loadImagesFunc load;
    getImageFunc get;
    updateImageFunc update;
    hasImageFunc has;
    getVersionFunc getVersion;
    getListImagesFunc getList;

    QLibrary *lib;
};

#endif // IMAGELIBRARY_H
