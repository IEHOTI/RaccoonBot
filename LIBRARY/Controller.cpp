#include "Controller.h"

Controller::Controller(QObject *parent) : QObject(parent) {
    m_main = 0;
    m_game = 0;
    m_rect = { 0,0,0,0 };
    mainPath = ":/pages";
}

void Controller::findObject(const Mat *finder, ErrorList *result) {
    if(finder != nullptr) finder->copyTo(m_mask);
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    if (m_mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "findObject->mask";
        return;
    }
    // Преобразуем изображение в оттенки серого
    Mat grayImage;
    cvtColor(m_mask, grayImage, COLOR_BGR2GRAY);
    // Бинаризуем изображение
    Mat binaryImage;
    threshold(grayImage, binaryImage, 127, 255, THRESH_BINARY_INV);
    // Находим контуры
    vector<vector<Point>> contours;
    findContours(binaryImage, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    // Находим прямоугольник, описывающие контур
    m_rect = boundingRect(contours[0]);
    if (m_rect.width == 0 || m_rect.height == 0) {
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "empty area of find";
        return;
    }
}

void Controller::compareObject(double rightVal, const Mat *object, const Mat *sample, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    Mat img1, sample1;

    if (object != nullptr) object->copyTo(img1);
    else m_object.copyTo(img1);
    if (img1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->object && m_object";
        return;
    }
    if (sample != nullptr) sample->copyTo(sample1);
    else m_sample.copyTo(sample1);
    if (sample1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->sample && m_sample";
        return;
    }

    m_rect.x = 0;
    m_rect.y = 0;
    cvtColor(img1, img1, COLOR_BGR2GRAY);
    cvtColor(sample1, sample1, COLOR_BGR2GRAY);

    // Создание матрицы результата
    int res_col = sample1.cols - img1.cols + 1;
    int res_row = sample1.rows - img1.rows + 1;
    Mat resultMat(res_row, res_col, CV_32FC1);

    // Сравнение шаблона с изображением
    matchTemplate(img1, sample1, resultMat, TM_SQDIFF_NORMED);

    // Нахождение наилучшего совпадения
    double minVal = 0;
    Point minLoc;
    minMaxLoc(resultMat, &minVal, nullptr, &minLoc, nullptr);
    emit errorLogging("==MatchTemplate: Result[" + QString::number(minVal) + "]; RightVal[" + QString::number(rightVal) + "]");

    // Проверка наилучшего совпадения
    if (minVal <= rightVal) {
        m_rect.x = minLoc.x;
        m_rect.y = minLoc.y;
        return;
    }

    observer.value.warning = m_Warning::FAIL_COMPARE;
    observer.print = false;
    return;
}

void Controller::compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, ErrorList *result, bool shoot, double rightVal) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(shoot) Screenshot();
    if (m_object.empty()) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    //Нахождение координат обрезаемой области(findObject)
    emit errorLogging("Converting mask: " + (pagePath + "/" + maskPath));
    convertImage(QImage((mainPath + "/" + pagePath + "/" + maskPath + ".png")), &m_mask,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->mask" + (mainPath + "/" + pagePath + "/" + maskPath + ".png");
        return;
    }

    findObject(nullptr,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    emit errorLogging("Converting sample: " + (pagePath + "/" + samplePath));
    convertImage(QImage((mainPath + "/" + pagePath + "/" + samplePath + ".png")), &m_sample,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->sample" + (mainPath + "/" + pagePath + "/" + samplePath + ".png");
        return;
    }

    Mat img1 = m_sample(m_rect);
    compareObject(rightVal,&img1,&m_object,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
}

void Controller::saveImage(const QString &savePath, const Mat &saveImage, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(!imwrite(savePath.toStdString(),saveImage)) {
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.comment = ("save image, path: " + savePath);
        return;
    }
}

void Controller::Screenshot() {
    int width = 900;
    int height = 600;
    HDC hdcWindow = GetDC(m_game);
    Mat res(height, width, CV_8UC4);
    RECT rc;
    GetClientRect(m_game, &rc);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP bitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ OBJ = SelectObject(hdcMem, bitmap);
    SelectObject(hdcMem, bitmap);
    BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY);
    SelectObject(hdcMem, OBJ);
    HDC hdcRar = GetDC(NULL);
    SelectObject(hdcRar, bitmap);
    StretchBlt(hdcRar, 0, 0, width, height, hdcMem, 0, 0, width, height, SRCCOPY);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, height, 1, 32 };
    GetDIBits(hdcMem, bitmap, 0, height, res.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    DeleteObject(bitmap);
    DeleteDC(hdcMem);
    DeleteDC(hdcRar);
    ReleaseDC(m_game, hdcWindow);
    flip(res, m_object, 0);
}

void Controller::convertImage(const QImage &imageOne, Mat *imageTwo, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    // Проверка входных данных
    if (imageOne.isNull()) {
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.print = false;
        return;
    }
    // Проверка указателя
    if (!imageTwo) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.print = false;
        return;
    }
    // Определяем формат QImage и конвертируем в соответствующий Mat
    Mat cvImg;
    switch (imageOne.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        //cvImg = Mat(imageOne.height(), imageOne.width(), CV_8UC4, (void*)imageOne.bits(), imageOne.bytesPerLine());
        //cvtColor(cvImg, cvImg, COLOR_RGBA2BGR);
        cvImg = Mat(imageOne.height(), imageOne.width(), CV_8UC4, (void*)imageOne.constBits(), imageOne.bytesPerLine());
        cvtColor(cvImg, cvImg, COLOR_RGBA2RGB);
        break;
    case QImage::Format_Grayscale8:
        cvImg = Mat(imageOne.height(), imageOne.width(), CV_8UC1, (void*)imageOne.bits(), imageOne.bytesPerLine());
        break;
    default:
        observer.value.warning = m_Warning::UNKNOWN;
        observer.comment = "unsupported format";
        return;
    }
    imageTwo->release();
    // Копируем результат
    cvImg.copyTo(*imageTwo);
}

Mat Controller::cutImage() {
    return m_object(m_rect);
}

Mat Controller::getMatObject() {
    return m_object;
}

void Controller::setMatObject(const Mat &image, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_object.release();
    image.copyTo(m_object);

    if(m_object.empty()){
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMatObject->Mat";
        return;
    }
}

void Controller::changeColor(const Mat &before, Mat *after, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    if (before.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "before";
        return;
    }
    // Делаем копию, чтобы не трогать оригинал
    cv::Mat temp = before.clone();
    cv::cvtColor(temp, temp, cv::COLOR_RGB2BGR);

    // Диапазоны (BGR!)
    cv::Scalar lowerWhite(200, 200, 200);
    cv::Scalar upperWhite(255, 255, 255);

    cv::Scalar lowerBlackBlue(0, 0, 0);
    cv::Scalar upperBlackBlue(100, 100, 100);

    cv::Scalar lowerBlue(100, 150, 0);
    cv::Scalar upperBlue(255, 255, 100);

    cv::Scalar lowerNumberBlue(105, 95, 15);
    cv::Scalar upperNumberBlue(165, 160, 130);

    cv::Scalar lowerYellow(15,95,105);
    cv::Scalar upperYellow(75,135,140);

    // Цвета замены (BGR!)
    cv::Scalar dullGreen(144, 238, 144);
    cv::Scalar white(255, 255, 255);

    cv::Mat mask;

    // Белое → зеленое
    cv::inRange(temp, lowerWhite, upperWhite, mask);
    temp.setTo(dullGreen, mask);

    // * → белое
    cv::Mat mask1, mask2, mask3, mask4;
    cv::inRange(temp, lowerBlackBlue, upperBlackBlue, mask1);
    cv::inRange(temp, lowerBlue, upperBlue, mask2);
    cv::inRange(temp, lowerNumberBlue, upperNumberBlue, mask3);
    cv::inRange(temp, lowerYellow, upperYellow, mask4);

    cv::bitwise_or(mask1, mask2, mask);

    cv::bitwise_or(mask, mask3, mask);
    cv::bitwise_or(mask, mask4, mask);

    temp.setTo(white, mask);

    temp.copyTo(*after);

    if (after->empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "after";
        return;
    }
}

void Controller::setSample(const Mat &sample, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_sample.release();
    sample.copyTo(m_sample);

    if(m_sample.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setSample->mat";
        return;
    }
}

void Controller::setSample(const QString &sample, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_sample.release();
    QString path = QDir::cleanPath(mainPath + "/" + sample + ".png");
    QImage image(path);
    if(image.isNull()){
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.comment = "setSample->path: " + path;
        return;
    }
    convertImage(image, &m_sample,result);
}

void Controller::setMask(const Mat &mask, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_mask.release();
    mask.copyTo(m_mask);
    if(m_mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMask->mat";
        return;
    }
}

void Controller::setMask(const QString &mask, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_mask.release();
    QString path = QDir::cleanPath(mainPath + "/" + mask + ".png");
    QImage image(path);
    if(image.isNull()){
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.comment = "setMask->path: " + path;
        return;
    }
    convertImage(image, &m_mask, result);
}

Rect& Controller::getRect() {
    return m_rect;
}

void Controller::isEmpty(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(m_main == NULL || m_game == NULL) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }
}

void Controller::isValidSize(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    RECT temp;
    int width,height;
    GetWindowRect(m_game,&temp);
    width = temp.right - temp.left;
    height = temp.bottom - temp.top;
    if(width != 900 || height != 600) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_SIZE;
        return;
    }
}

void Controller::isValidPos(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    RECT temp;
    GetWindowRect(m_game,&temp);
    if(temp.left != 1 || temp.top != 33) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_POS;
        return;
    }
}

void Controller::click(ErrorList *result, int count, int delay) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();//??
    Mat before,after;
    m_object.copyTo(before);
    Rect click = m_rect;
    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(click.x + 5, click.y + 5));
        Sleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(click.x + 5, click.y + 5));
        Sleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after, &before, &l_result);
        if (!l_result) return;
        Sleep(delay);
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = ("bot pos: " + QString::number(m_rect.x) + ";" + QString::number(m_rect.y) + "]");
    return;
}

void Controller::clickPosition(const Rect &point, ErrorList *result, int count, int delay) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();//??
    Mat before,after;
    m_object.copyTo(before);
    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(point.x + 5, point.y + 5));
        Sleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(point.x + 5, point.y + 5));
        Sleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after, &before, &l_result);
        if (!l_result) return;
        Sleep(delay);
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = ("user pos: " + QString::number(m_rect.x) + ";" + QString::number(m_rect.y) + "]");
    return;
}

void Controller::clickSwipe(const Rect &start, const Rect &finish, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    Screenshot();//??
    Mat before,after;
    m_object.copyTo(before);

    auto l_start = Vec2i(start.x, start.y);
    auto l_stop = Vec2i(finish.x, finish.y);
    auto path = l_stop - l_start;
    Vec2f path_dir = path;
    path_dir = normalize(path_dir);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    BYTE arr[256];
    memset(arr, 0, sizeof(256));
    GetKeyboardState(arr);
    auto old = arr[VK_LBUTTON];
    arr[VK_LBUTTON] = 128;
    SetKeyboardState(arr);

    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(l_start[0], l_start[1]));
    Sleep(15);

    const int count = int(norm(path));
    Vec2i last_pos;

    for (int i = 0; i < count; ++i)
    {
        auto pos = Vec2i(path_dir  *float(i));
        last_pos = l_start + pos;

        SetKeyboardState(arr);
        PostMessage(m_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(last_pos[0], last_pos[1]));

        const int pause = 50;
        const int offset = 15;
        if (i < offset) {
            auto p = 10 + ((offset - (i + 1))  *pause) / offset;
            Sleep(p);
        }
        else if (i > count - offset) {
            auto p = 30 + ((offset - (count - i))  *pause) / offset;
            Sleep(p);
        }
        else if (i % 5 == 0) Sleep(1);
    }

    PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(last_pos[0], last_pos[1]));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    Sleep(15);

    arr[VK_LBUTTON] = old;
    SetKeyboardState(arr);

    Screenshot();
    m_object.copyTo(after);
    compareObject(0, &after, &before, &l_result);
    if (!l_result) return;
    else {
        observer.value.warning = m_Warning::FAIL_CLICK;
        observer.comment = ("swipe start: " + QString::number(start.x) + ";" + QString::number(start.y) + "] swipe finish: " + QString::number(finish.x) + ";" + QString::number(finish.y) + "]");
        return;
    }

}

void Controller::clickButton(const QString &pagePath, const QString &buttonName, ErrorList *result, int count, int delay) {
    emit errorLogging("=Клик по кнопке:\"" + mainPath + "/" + pagePath + "/" + buttonName + ".png" + "\"");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    m_mask.release();
    convertImage(QImage((mainPath + "/" + pagePath + "/" + buttonName + ".png")), &m_mask,&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.comment = "convert->mask";
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    click(result,count,delay);
}
void Controller::clickMapButton(const QString &mapName, const QString &buttonName, ErrorList *result, int count, int delay) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    checkMap(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    Rect start = {690,290,0,0};
    Rect finish = {100,290,0,0};
    while (x < 2){//right swipe
        compareSample("map",mapName,buttonName,&l_result,true);
        if(!l_result) {
            clickSwipe(start,finish);
            x++;
        }
        else break;
    }
    if(!l_result) {//left swipe
        x = 0;
        while(x < 4){
            compareSample("map",mapName,buttonName,&l_result,true);
            if(!l_result) {
                clickSwipe(finish,start);
                x++;
            }
            else break;
        }
    }
    if(!l_result) {
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "Cant find " + buttonName;
        return;
    }
    else click(result,count,delay);
}

void Controller::clickEsc(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_KEYDOWN, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
    Sleep(100);
    PostMessage(m_game, WM_KEYUP, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
    Sleep(500);
    Screenshot();
    m_object.copyTo(after);
    compareObject(0, &after,&before,&l_result);
    if(!l_result) return;
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = "Esc";
    return;
}

void Controller::clickReturn(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_KEYDOWN, VK_RETURN, 0);
    Sleep(100);
    PostMessage(m_game, WM_KEYUP, VK_RETURN, 0);
    Sleep(500);
    Screenshot();
    m_object.copyTo(after);
    compareObject(0.0001, &after,&before,&l_result);
    if(!l_result) return;
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = "Return-Enter";
    return;
}

//void Controller::Initialize(userProfile *user, ErrorList *result){}

void Controller::userInitialize(userProfile *user, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    clickButton("main","button_user",&l_result,3);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    do {
        compareSample("user","sample","compare",&l_result,true);
        if(!l_result) {
            x++;
            Sleep(1000);
        }
        else break;
    } while(x < 10);
    if(!l_result) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "dont open user profile";
        return;
    }
    setMask("user/user_id",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    emit Recognize(cutImage(),user->user_ID);
    if(user->user_ID <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_id";
        return;
    }
    //////
    user->subscribe = typeSub::admin;

    //////
    setMask("user/user_power",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    emit Recognize(cutImage(),user->history_power);
    if(user->history_power <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_power";
        return;
    }
    //////
    user->state_premium = true;
    user->state_ads = false;
    user->count_units = 6;//потом сделать адекватное распознавание после получения инфы о подписке, условно
    user->leftover_time = "9999999 days for Raccoons";
    clickButton("user","button_close");
}

void Controller::getGameError() {
    if(m_object.empty()) emit errorLogging("Пустой m_object");
    else imshow("1",m_object);
    if(m_mask.empty()) emit errorLogging("Пустой m_mask");
    else imshow("2",m_mask);
    if(m_sample.empty()) emit errorLogging("Пустой m_sample");
    else imshow("3",m_sample);
}

QString& Controller::getMainPath() {
    return mainPath;
}

void Controller::setMainPath(const QString &path) {
    mainPath.clear();
    mainPath = path;
}

void Controller::Start(userProfile *user, ErrorList *result) {
    emit Logging("Бот запущен");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(!FindEmulator(user->emulator_name,&m_main,&m_game)) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "find emulator";
        return;
    }
    char name[256];
    GetClassNameA(m_main, name, sizeof(name));
    if (strcmp(name, "LDPlayerMainFrame") == 0) {
        user->emulatorType = typeEmu::ld_player;
        Emulator *emulator = new LDPlayer(this);
        emulator->Initialize(&m_main);
        emit emulatorCreated(emulator);
    }
    else {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "recognize emulator type";
        return;
    }
    do{
        checkPreMainPage();
        checkMainPage(&l_result);
        if(!l_result){
            fixGameError(&l_result);
            if(!l_result){
                observer.value.error = m_Error::FAIL_INIT;
                observer.comment = "find mainPage";
                return;
            }
        }
    }while(!l_result);
    do{
        checkSettings(&l_result);
        if(!l_result){
            fixGameError(&l_result);
            if(!l_result){
                observer.value.error = m_Error::FAIL_INIT;
                observer.comment = "find settings";
                return;
            }
        }
    }while(!l_result);
    userInitialize(user,&l_result);
    if(!l_result) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.print = false;
        return;
    }
    //refreshMainPage(&l_result); // poka chto
    // if(!l_result) {
    //     err.value = false;
    //     err.errorMessage = "Не получилось сбросить положение экрана(.";
    //     return;
    // }
    //пока что временно это всё, потом добавить
    emit endStart();
}

void Controller::Stop(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ///
    if(m_main == NULL) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        observer.comment = "m_main null";
        return;
    }
    PostMessage(m_main,WM_CLOSE,0,0);
    m_main = 0;
    m_game = 0;
}

void Controller::LocalLogging(const QString &msg) {
    //getGameError();//пока что пустой
    emit errorLogging(msg);
}

void Controller::checkLoading() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do{
        compareSample("load","sample","compare",&l_result,true);
        if(!l_result) break;
        else Sleep(1000);
    } while(true);
    Sleep(500);
}

void Controller::checkGameLoading() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        compareSample("load","sample_open","compare_open",&l_result,true);
        if(!l_result) break;
        else Sleep(1000);
    } while(true);
    do {
        compareSample("load","sample_logo","compare_logo",&l_result,true);
        if(!l_result) break;
        else Sleep(1000);
    } while(true);
    checkLoading();
}

void Controller::checkPreMainPage() {
    checkGameLoading();
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    checkEvent(&l_result);
    if(l_result) skipEvent();
    compareSample("load", "sample_mail", "compare_mail", &l_result,true);
    if (l_result) {
        clickButton("load","button_close_mail",&l_result);
        Sleep(500);
    }

    //ne pomny gde pass viskakivaet + сделать выходы из циклов
    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
    do {
        compareSample("load", "sample_offer", "compare_offer", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
}
///////////
void Controller::checkMainPage(ErrorList *result) {
    Sleep(500);
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    compareSample("main","sample","compare",&l_result,true);
    if(!l_result){
        emit errorLogging("Первая проверка провалена.");
        compareSample("main","sample","compare_1",&l_result,true);
        if(!l_result){
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "fail 2x check main page";
            return;
        }
    }
}

void Controller::checkEvent(ErrorList *result) {
    //if(!event) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    int x = 0;
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    setMask("event/compare",&l_result);
    if(!l_result) {
        observer.value = l_result;
        return;
    }
    convertImage(QImage((mainPath + "/event/sample.png")), &m_object,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.comment = "wrong convert - /event/sample.png";
        return;
    }
    findObject();
    Mat find = cutImage();
    do {
        Screenshot();
        compareObject(0.02,&find,&m_object,&l_result);
        if(l_result) break;
        else {
            x++;
            Sleep(500);
        }
    } while(x < 2);
    if(x == 2) {
        observer.value.warning = m_Warning::NO_EVENT;
        observer.print = false;
        return;
    }
}

void Controller::checkSettings(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    clickButton("main","button_settings",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    do {
        compareSample("settings","sample","compare",&l_result,true);
        if(l_result) break;
        else {
            x++;
            Sleep(500);
        }
    } while(x < 10);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    compareSample("settings","sample","state_fps",&l_result);
    if(!l_result) clickButton("settings","button_fps",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    compareSample("settings","sample","state_lang",&l_result);
    if(!l_result) {
        clickButton("settings","button_lang",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        Sleep(1000);
        compareSample("settings","sample_change_lang","compare_change_lang",&l_result,true);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        clickButton("settings","button_en",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        compareSample("settings","sample_confirm","compare_confirm",&l_result,true);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        clickButton("settings","button_yes",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
    } else clickEsc();
}

void Controller::refreshMainPage(ErrorList *result) {
    // emit errorLogging("===Обновление главной страницы===");
    // m_error err(result);
    // connect(&err, &m_Error::Logging, this, &Controller::LocalLogging);
    // ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    // checkMainPage(&l_result);
    // if(!l_result) {
    //     err.value = false;
    //     err.errorMessage = "Ошибка: не на главное странице.";
    //     return;
    // }
    // clickButton("main","button_friends",&l_result);
    // if(!l_result){
    //     err.value = false;
    //     err.errorMessage = "Ошибка: не удалось нажать на кнопку друзей.";
    //     return;
    // }
    // int x = 0;
    // do {
    //     compareSample("top_players","sample","compare",&l_result,true);
    //     if(l_result) break;
    //     else {
    //         x++;
    //         Sleep(500);
    //     }
    // } while(x < 100);
    // if(x == 100) {
    //     err.value = false;
    //     err.errorMessage = "Ошибка: не прогрузился список топ-игроков.";
    //     return;
    // }
    // Sleep(500);//?
    // clickPosition(Rect(480,200,0,0));
    // Sleep(500);
    // clickPosition(Rect(480,200,0,0));
    // do {
    //     compareSample("top_players","sample_top","compare_top",&l_result,true);
    //     Sleep(500);
    // } while(l_result);
    // clickEsc();
    // checkMainPage(&l_result);
    // if(!l_result){
    //     err.value = false;
    //     err.errorMessage = "Ошибка сброса положения экрана.";
    //     return;
    // }
}

void Controller::skipEvent() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        checkEvent(&l_result);
        if(l_result) {
            clickEsc();
            Sleep(1000);
        }
    } while(l_result);
}

void Controller::checkMap(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    do {
        compareSample("map","sample","compare",&l_result,true);
        if(!l_result) {
            x++;
            Sleep(1000);
        }
        else return;
    } while (x < 15);
    observer.value.warning = m_Warning::FAIL_CHECK;
    observer.comment = "map";
    return;
}

void Controller::fixPopUpError(ErrorList *result){
    emit errorLogging("===Фикс всплывающих окон===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    QList<QString> temp;
    temp << "device" << "sleep" << "daily" ;
    for(int i = 0; i < temp.size(); i++){
        compareSample("warnings/general","sample_"+temp[i],"compare_"+temp[i],&l_result,true);
        if(l_result){
            clickButton("warnings/general","button_" + temp[i],&l_result,2);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            compareSample("load","sample","compare",&l_result,true);
            if(l_result){
                checkLoading();
                return;
            }
        }
    }
    Sleep(1000);
    compareSample("warnings/general","sample_goblin","compare_goblin",&l_result,true);
    if(l_result){
        //тут проверка будет на рекламы, но пока что тупо Esc
        clickEsc();
        return;
    }
}

void Controller::fixGameError(ErrorList *result) {
    if(result && *result) return;
    emit errorLogging("===Фикс полный===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    fixPopUpError(&l_result);
    if(l_result) return;
    //тут будет проверка и фикс всплывающей фигни "ваш замок уязвим" if(l_result) return;
    //тут фикс размера эмуля после перезапуска
    //если дошло до сюда тут сигнал послать чтобы он перезапустил эмуль emit endEmu emit startEmu в GUI
    //связать Emulator и this
    do{
        compareSample("load","sample","compare",&l_result,true);
    }while(!l_result);
    checkLoading();
    checkPreMainPage();
    checkMainPage();
    return;
}

void Controller::findBarracks(ErrorList *result) {}

void Controller::entryBarracks(ErrorList *result) {}

void Controller::scanSquadCount(userProfile *user, ErrorList *result) {}
