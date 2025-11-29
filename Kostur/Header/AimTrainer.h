#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <chrono>
#include "SimpleFont.h"

struct Target {
    float x, y;
    float radius;
    float lifeTime;
    float maxLifeTime;
    bool active;
};

struct Button {
    float x, y;
    float width, height;
    bool isHovered;
};

class AimTrainer {
private:
    unsigned int shaderProgram;
    unsigned int textShaderProgram;
    unsigned int textureShaderProgram;
    unsigned int VAO, VBO;
    unsigned int textVAO, textVBO;
    unsigned int textureVAO, textureVBO;
    unsigned int studentInfoTexture;
    SimpleFont* font;
    
    std::vector<Target> targets;
    Button restartButton;
    Button exitButton;
    int score;
    int lives;
    int maxLives;
    double startTime;
    double lastHitTime;
    double totalHitTime;
    int hitCount;
    bool gameOver;
    float spawnTimer;
    float spawnInterval;
    float initialSpawnInterval;
    float minSpawnInterval;
    float targetLifeTimeMultiplier;
    float minTargetLifeTime;
    int windowWidth;
    int windowHeight;
    
    double gameOverTime;
    double survivalTime;
    double avgHitSpeed;
    bool exitRequested;
    
    void initBuffers();
    void spawnTarget();
    void updateDifficulty();
    void drawCircle(float x, float y, float radius, float r, float g, float b);
    void drawText(float x, float y, const char* text, float scale);
    void drawRect(float x, float y, float width, float height, float r, float g, float b);
    void drawTexture(float x, float y, float width, float height, unsigned int texture);
    bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh);
    
public:
    AimTrainer(int width, int height);
    ~AimTrainer();
    
    void update(float deltaTime);
    void render();
    void handleMouseClick(double mouseX, double mouseY);
    void restart();
    bool isGameOver() const { return gameOver; }
    bool shouldExit() const;
};
