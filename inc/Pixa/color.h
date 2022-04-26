#ifndef PIXA_COLOR_H
#define PIXA_COLOR_H

#include <stdint.h>

#define COLOR_GREY                  (Color) {192, 192, 192, 255}
#define COLOR_DARK_GREY             (Color) {128, 128, 128, 255}
#define COLOR_VERY_DARK_GREY        (Color) { 64,  64,  64, 255}
#define COLOR_RED                   (Color) {255,   0,   0, 255}
#define COLOR_DARK_RED              (Color) {128,   0,   0, 255}
#define COLOR_VERY_DARK_RED         (Color) {64,    0,   0, 255}
#define COLOR_YELLOW                (Color) {255, 255,   0, 255}
#define COLOR_DARK_YELLOW           (Color) {128, 128,   0, 255}
#define COLOR_VERY_DARK_YELLOW      (Color) { 64,  64,   0, 255}
#define COLOR_GREEN                 (Color) {  0, 255,   0, 255}
#define COLOR_DARK_GREEN            (Color) {  0, 128,   0, 255}
#define COLOR_VERY_DARK_GREEN       (Color) {  0,  64,   0, 255}
#define COLOR_CYAN                  (Color) {  0, 255, 255, 255}
#define COLOR_DARK_CYAN             (Color) {  0, 128, 128, 255}
#define COLOR_VERY_DARK_CYAN        (Color) {  0,  64,  64, 255}
#define COLOR_BLUE                  (Color) {  0,   0, 255, 255}
#define COLOR_DARK_BLUE             (Color) {  0,   0, 128, 255}
#define COLOR_VERY_DARK_BLUE        (Color) {  0,   0,  64, 255}
#define COLOR_MAGENTA               (Color) {255,   0, 255, 255}
#define COLOR_DARK_MAGENTA          (Color) {128,   0, 128, 255}
#define COLOR_VERY_DARK_MAGENTA     (Color) { 64,   0,  64, 255}
#define COLOR_WHITE                 (Color) {255, 255, 255, 255}
#define COLOR_BLACK                 (Color) {  0,   0,   0, 255}
#define COLOR_BLANK                 (Color) {  0,   0,   0,   0}

typedef struct {
    uint8_t r;
    uint8_t g; 
    uint8_t b;
    uint8_t a;
} Color;

Color color_hsv(int h, int s, int v);

#endif