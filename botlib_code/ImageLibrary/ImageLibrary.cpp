#include "ImageLibrary.h"
#include <QString>
#include <QLibrary>
#include "CustomError/Exception.h"

ImageLibrary::ImageLibrary(const QString &name) {
    lib = new QLibrary(name);
    if(!lib->load()) throw ImageException("Cannot load library");

    load = (loadImagesFunc)lib->resolve("loadImages");
    get = (getImageFunc)lib->resolve("getImage");
    update = (updateImageFunc)lib->resolve("updateImage");
    has = (hasImageFunc)lib->resolve("hasImage");
    getVersion = (getVersionFunc)lib->resolve("getVersion");
    getList = (getListImagesFunc)lib->resolve("getListImages");

    if(!load || !get || !update || !has || !getVersion) throw ImageException("Cannot load one of func");
}

ImageLibrary::~ImageLibrary() {
    load = nullptr;
    get = nullptr;
    update = nullptr;
    has = nullptr;
    getVersion = nullptr;
    lib->unload();
}
