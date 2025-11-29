#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <chrono>

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
    unsigned int VAO, VBO;
    unsigned int textVAO, textVBO;
    
    std::vector<Target> targets;
    Button restartButton;
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
    int windowWidth;
    int windowHeight;
    
    double gameOverTime;
    double survivalTime;
    double avgHitSpeed;
    
    void initBuffers();
    void spawnTarget();
    void drawCircle(float x, float y, float radius, float r, float g, float b);
    void drawText(float x, float y, const char* text, float scale);
    void drawRect(float x, float y, float width, float height, float r, float g, float b);
    bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh);
    
public:
    AimTrainer(int width, int height);
    ~AimTrainer();
    
    void update(float deltaTime);
    void render();
    void handleMouseClick(double mouseX, double mouseY);
    void restart();
    bool isGameOver() const { return gameOver; }
};
