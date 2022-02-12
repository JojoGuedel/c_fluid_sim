#ifndef PIXA_LOG_H
#define PIXA_LOG_H

#define LOG_INFO        0
#define LOG_WARNING     1
#define LOG_ERROR       2
#define LOG_NONE        3

// #define LOG_USER        "User"
// #define LOG_GLFW        "GLFW"
// #define LOG_OPENGL      "OpenGL"

#ifndef LOG_LEVEL
#ifdef DEBUG
#define LOG_LEVEL LOG_INFO
#else
#define LOG_LEVEL LOG_ERROR
#endif
#endif

#if LOG_LEVEL <= LOG_INFO
#define log_info(...) log_msg("INFO", "USER", __VA_ARGS__)
#else
#define log_info(...)
#endif

#if LOG_LEVEL <= LOG_WARNING
#define log_warning(...) log_msg("WARNING", "USER", __VA_ARGS__)
#else
#define log_warning(...)
#endif

#if LOG_LEVEL <= LOG_ERROR
#define log_error(...) log_msg("ERROR", "USER", __VA_ARGS__)
#else
#define log_error(...)
#endif

void log_msg(const char *msg_t, const char *sender, const char *msg_format, ...);

#endif