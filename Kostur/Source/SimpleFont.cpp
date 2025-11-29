#include "../Header/SimpleFont.h"
#include <cmath>

SimpleFont::SimpleFont(unsigned int shader, int width, int height) 
    : shaderProgram(shader), windowWidth(width), windowHeight(height)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

SimpleFont::~SimpleFont() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void SimpleFont::drawSegment(float x, float y, float w, float h) {
    glBindVertexArray(VAO);
    
    float vertices[] = {
        x, y,
        x + w, y,
        x + w, y + h,
        x, y,
        x + w, y + h,
        x, y + h
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SimpleFont::drawDigit(int digit, float x, float y, float size) {
    float w = size * 0.6f;
    float h = size;
    float t = size * 0.15f;
    
    glUseProgram(shaderProgram);
    
    int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
    
    bool segments[10][7] = {
        {1,1,1,1,1,1,0}, // 0
        {0,1,1,0,0,0,0}, // 1
        {1,1,0,1,1,0,1}, // 2
        {1,1,1,1,0,0,1}, // 3
        {0,1,1,0,0,1,1}, // 4
        {1,0,1,1,0,1,1}, // 5
        {1,0,1,1,1,1,1}, // 6
        {1,1,1,0,0,0,0}, // 7
        {1,1,1,1,1,1,1}, // 8
        {1,1,1,1,0,1,1}  // 9
    };
    
    if (digit < 0 || digit > 9) return;
    
    if (segments[digit][0]) drawSegment(x, y, w, t);
    if (segments[digit][1]) drawSegment(x + w - t, y, t, h * 0.5f);
    if (segments[digit][2]) drawSegment(x + w - t, y + h * 0.5f, t, h * 0.5f);
    if (segments[digit][3]) drawSegment(x, y + h - t, w, t);
    if (segments[digit][4]) drawSegment(x, y + h * 0.5f, t, h * 0.5f);
    if (segments[digit][5]) drawSegment(x, y, t, h * 0.5f);
    if (segments[digit][6]) drawSegment(x, y + h * 0.5f - t * 0.5f, w, t);
}

void SimpleFont::drawNumber(int number, float x, float y, float size) {
    if (number == 0) {
        drawDigit(0, x, y, size);
        return;
    }
    
    int digits[10];
    int count = 0;
    int temp = abs(number);
    
    while (temp > 0) {
        digits[count++] = temp % 10;
        temp /= 10;
    }
    
    float spacing = size * 0.7f;
    
    for (int i = count - 1; i >= 0; i--) {
        drawDigit(digits[i], x + (count - 1 - i) * spacing, y, size);
    }
}

void SimpleFont::drawChar(char c, float x, float y, float size) {
    if (c >= '0' && c <= '9') {
        drawDigit(c - '0', x, y, size);
    } else if (c == ':') {
        float dotSize = size * 0.2f;
        float spacing = size * 0.25f;
        glUseProgram(shaderProgram);
        int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
        drawSegment(x + spacing, y + size * 0.25f, dotSize, dotSize);
        drawSegment(x + spacing, y + size * 0.65f, dotSize, dotSize);
    }
}

void SimpleFont::drawText(const char* text, float x, float y, float size) {
    float currentX = x;
    float digitSpacing = size * 0.8f;
    float colonSpacing = size * 0.5f;
    
    for (int i = 0; text[i] != '\0'; i++) {
        drawChar(text[i], currentX, y, size);
        if (text[i] == ':') {
            currentX += colonSpacing;
        } else {
            currentX += digitSpacing;
        }
    }
}
