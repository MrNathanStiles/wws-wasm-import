
#ifndef ACTRWASM_H
#define ACTRWASM_H
#include "actralloc.h"

// basic header required for integration with the ActR platform
// all actr_canvas2d prefixed method will follow html CanvasRenderingContext2d as closly as possible
// see docs at https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D

extern void _actr_sanity_size(int intSize, int longSize, int floatSize, int doubleSize, int stateSize, void * state);
extern int actr_authenticated();
// end helpers
struct ActrPoint
{
    int x;
    int y;
};

struct ActrSize
{
    int w;
    int h;
};

struct ActrState
{
    // canvasSize will be updated automatically
    struct ActrSize canvasSize;
    // pointer position will be updated automatically
    struct ActrPoint pointerPosition;
    // text size will be updated when actr_canvas2d_measureText is called
    struct ActrPoint textSize;
    int debug;
};

struct ActrState * actrState;

unsigned int actr_pack_bytes(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    return (r << 24) | (g << 16) | (b << 8) | a;
}
void actr_unpack_bytes(unsigned int value, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
    *r = (value >> 24) & 0xFF;
    *g = (value >> 16) & 0xFF;
    *b = (value >> 8) & 0xFF;
    *a = value & 0xFF;
}
/// @brief internal use
[[clang::export_name("_actr_sanity")]]
void _actr_sanity()
{
    actrState = actr_malloc(sizeof(struct ActrState));
    _actr_sanity_size(sizeof(int), sizeof(long long), sizeof(float), sizeof(double), sizeof(struct ActrState), actrState);
}

/// @brief get length of null terminated string
/// @param string 
/// @return length of the string
unsigned long strlen(const char * string) {
    int result = 0;
    while (*string++) {
        result++;
    }
    return result;
}

char *substr(char * text, int start, int length) {
    if (length == 0) {
        length = strlen(text) - start;
    }
    char * newText = actr_malloc(length + 1);

    for (int i = 0; i < length; i++) {
        newText[i] = text[start + i]; 
    }
    return newText;
}
void actr_heap_string(char ** target, char *text)
{
    if (*target != 0)
    {
        actr_free(*target);
    }
    int size = strlen(text);
    char *result = actr_malloc(size + 1);
    for (int i = 0; i < size; i++) {
        result[i] = text[i];
    }
    *target = result;
}
#endif
