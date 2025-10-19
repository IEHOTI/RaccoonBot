#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

struct StopException : public std::exception {
    const char *what() const noexcept override {
        return "Thread was stopped by request";
    }
};

struct PauseException : public std::exception {
    const char *what() const noexcept override {
        return "Thread was paused by request";
    }
};

struct ImageException : public std::exception {
    ImageException() : msg("ImageLib problem") {}
    ImageException(const std::string &message) : msg(message) {}

    const char *what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

#endif // EXCEPTION_H
