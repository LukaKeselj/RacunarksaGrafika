#pragma once
#include <GL/glew.h>

class SimpleFont {
private:
    unsigned int shaderProgram;
    unsigned int VAO, VBO;
    int windowWidth, windowHeight;
    
    void drawSegment(float x, float y, float w, float h);
    
public:
    SimpleFont(unsigned int shader, int width, int height);
    ~SimpleFont();
    
    void drawDigit(int digit, float x, float y, float size);
    void drawNumber(int number, float x, float y, float size);
    void drawText(const char* text, float x, float y, float size);
    void drawChar(char c, float x, float y, float size);
};
