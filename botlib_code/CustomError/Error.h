#ifndef ERROR_H
#define ERROR_H

enum class m_Error {
    NO_ERR,
    UNKNOWN,
    //image
    EMPTY_IMG,
    WRONG_IMG_PATH,
    //emulator
    FAIL_INIT,
    NO_ACTIVE_EMULATOR,
    //other
    STOP_TASK,
    PAUSE_TASK,
};

#endif // ERROR_H
