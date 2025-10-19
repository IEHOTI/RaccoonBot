#include "Controller/Controller.h"

cv::Rect& Controller::getRect() { return m_rect; }

QString& Controller::getMainPath() { return mainPath; }

void Controller::setMainPath(const QString &path) {
    mainPath.clear();
    mainPath = path;
}
