#include "../Header/AimTrainer.h"
#include "../Header/Util.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

AimTrainer::AimTrainer(int width, int height) 
    : score(0), lives(3), maxLives(3), gameOver(false), spawnTimer(0.0f), 
      spawnInterval(1.5f), initialSpawnInterval(1.5f), minSpawnInterval(0.5f),
      targetLifeTimeMultiplier(1.0f), minTargetLifeTime(1.0f),
      windowWidth(width), windowHeight(height), hitCount(0), totalHitTime(0.0),
      lastHitTime(0.0), gameOverTime(0.0), survivalTime(0.0), avgHitSpeed(0.0),
      font(nullptr), exitRequested(false)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    shaderProgram = createShader("Shaders/basic.vert", "Shaders/circle.frag");
    textShaderProgram = createShader("Shaders/text.vert", "Shaders/text.frag");
    textureShaderProgram = createShader("Shaders/texture.vert", "Shaders/texture.frag");
    
    font = new SimpleFont(textShaderProgram, windowWidth, windowHeight);
    
    studentInfoTexture = loadImageToTextureNoFlip("Resources/indeks.png");
    
    initBuffers();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
    float boxWidth = 400;
    float boxHeight = 330;
    float boxX = (windowWidth - boxWidth) / 2;
    float boxY = (windowHeight - boxHeight) / 2;
    
    restartButton.x = boxX + 80;
    restartButton.y = boxY + 200;
    restartButton.width = 240;
    restartButton.height = 50;
    restartButton.isHovered = false;
    
    exitButton.x = boxX + 80;
    exitButton.y = boxY + 260;
    exitButton.width = 240;
    exitButton.height = 50;
    exitButton.isHovered = false;
    
    for (int i = 0; i < 3; i++) {
        spawnTarget();
    }
}

AimTrainer::~AimTrainer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(textShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteTextures(1, &studentInfoTexture);
    delete font;
}

void AimTrainer::initBuffers() {
    float vertices[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    
    glBindVertexArray(0);
}

void AimTrainer::spawnTarget() {
    Target target;
    target.radius = 50.0f;
    
    target.x = target.radius + static_cast<float>(rand()) / RAND_MAX * (windowWidth - 2 * target.radius);
    target.y = target.radius + static_cast<float>(rand()) / RAND_MAX * (windowHeight - 2 * target.radius - 100);
    target.y += 50;
    
    target.maxLifeTime = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
    target.lifeTime = target.maxLifeTime;
    target.active = true;
    
    targets.push_back(target);
}

void AimTrainer::restart() {
    score = 0;
    lives = 3;
    gameOver = false;
    spawnTimer = 0.0f;
    hitCount = 0;
    totalHitTime = 0.0;
    gameOverTime = 0.0;
    survivalTime = 0.0;
    avgHitSpeed = 0.0;
    
    targets.clear();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
    for (int i = 0; i < 3; i++) {
        spawnTarget();
    }
    
    std::cout << "\n=== NOVA IGRA ===" << std::endl;
}

void AimTrainer::update(float deltaTime) {
    if (gameOver) return;
    
    spawnTimer += deltaTime;
    if (spawnTimer >= spawnInterval) {
        spawnTarget();
        spawnTimer = 0.0f;
    }
    
    for (auto& target : targets) {
        if (target.active) {
            target.lifeTime -= deltaTime;
            if (target.lifeTime <= 0.0f) {
                target.active = false;
                lives--;
                std::cout << "MISS! Zivoti preostali: " << lives << std::endl;
                
                if (lives <= 0) {
                    gameOver = true;
                    gameOverTime = glfwGetTime();
                    survivalTime = gameOverTime - startTime;
                    if (hitCount > 0) {
                        avgHitSpeed = totalHitTime / hitCount;
                    }
                }
            }
        }
    }
    
    targets.erase(
        std::remove_if(targets.begin(), targets.end(), 
            [](const Target& t) { return !t.active; }),
        targets.end()
    );
}

void AimTrainer::render() {
    glUseProgram(shaderProgram);
    
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    int projLoc = glGetUniformLocation(shaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    for (const auto& target : targets) {
        if (target.active) {
            drawCircle(target.x, target.y, target.radius, 1.0f, 0.3f, 0.3f);
        }
    }
    
    glUseProgram(textShaderProgram);
    projLoc = glGetUniformLocation(textShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    if (!gameOver) {
        double currentTime = glfwGetTime();
        double elapsed = currentTime - startTime;
        
        drawRect(10, 10, 550, 80, 0.0f, 0.0f, 0.0f);
        drawRect(12, 12, 546, 76, 0.2f, 0.2f, 0.2f);
        
        for (int i = 0; i < maxLives; i++) {
            if (i < lives) {
                drawRect(20 + i * 30, 20, 20, 20, 1.0f, 0.2f, 0.2f);
            } else {
                drawRect(20 + i * 30, 20, 20, 20, 0.3f, 0.3f, 0.3f);
            }
        }
        
        int minutes = static_cast<int>(elapsed) / 60;
        int seconds = static_cast<int>(elapsed) % 60;
        int centiseconds = static_cast<int>((elapsed - static_cast<int>(elapsed)) * 100) % 100;
        
        std::stringstream timeStr;
        timeStr << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ":" << (centiseconds < 10 ? "0" : "") << centiseconds;
        
        font->drawText(timeStr.str().c_str(), 130, 23, 30);
        
        double hitSpeed = 0.0;
        if (hitCount > 0) {
            hitSpeed = totalHitTime / hitCount;
        }
        
        std::cout << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ":" << (centiseconds < 10 ? "0" : "") << centiseconds
                  << " | Zivoti: " << lives << "/" << maxLives 
                  << " | Pogodaka: " << score << " | Avg Speed: " << hitSpeed << "s         \r" << std::flush;
    } else {
        float boxWidth = 400;
        float boxHeight = 330;
        float boxX = (windowWidth - boxWidth) / 2;
        float boxY = (windowHeight - boxHeight) / 2;
        
        drawRect(boxX, boxY, boxWidth, boxHeight, 0.0f, 0.0f, 0.0f);
        drawRect(boxX + 2, boxY + 2, boxWidth - 4, boxHeight - 4, 0.3f, 0.3f, 0.3f);
        
        drawRect(restartButton.x, restartButton.y, restartButton.width, restartButton.height, 0.2f, 0.8f, 0.2f);
        
        drawRect(exitButton.x, exitButton.y, exitButton.width, exitButton.height, 0.8f, 0.2f, 0.2f);
        
        static bool printedOnce = false;
        if (!printedOnce) {
            std::cout << "\n\n=== GAME OVER ===" << std::endl;
            std::cout << "Vreme preživljavanja: " << (int)survivalTime << "s" << std::endl;
            std::cout << "Ukupno pogodaka: " << score << std::endl;
            std::cout << "Prose?na brzina poga?anja: " << avgHitSpeed << "s" << std::endl;
            std::cout << "\nPritisni 'R' za restart ili klikni na zeleno dugme" << std::endl;
            std::cout << "Pritisni 'ESC' za izlaz ili klikni na crveno dugme" << std::endl;
            std::cout << "================\n" << std::endl;
            printedOnce = true;
        }
    }
}

void AimTrainer::drawCircle(float x, float y, float radius, float r, float g, float b) {
    glBindVertexArray(VAO);
    
    float model[16] = {
        radius, 0.0f, 0.0f, 0.0f,
        0.0f, radius, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f
    };
    
    int modelLoc = glGetUniformLocation(shaderProgram, "uModel");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
    
    int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3f(colorLoc, r, g, b);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AimTrainer::drawRect(float x, float y, float width, float height, float r, float g, float b) {
    glBindVertexArray(textVAO);
    
    float vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y,
        x + width, y + height,
        x, y + height
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    int colorLoc = glGetUniformLocation(textShaderProgram, "uColor");
    glUniform3f(colorLoc, r, g, b);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AimTrainer::handleMouseClick(double mouseX, double mouseY) {
    if (gameOver) {
        if (isPointInRect(static_cast<float>(mouseX), static_cast<float>(mouseY), 
                         restartButton.x, restartButton.y, restartButton.width, restartButton.height)) {
            restart();
        }
        else if (isPointInRect(static_cast<float>(mouseX), static_cast<float>(mouseY), 
                              exitButton.x, exitButton.y, exitButton.width, exitButton.height)) {
            exitRequested = true;
        }
        return;
    }
    
    double currentTime = glfwGetTime();
    
    bool hit = false;
    for (auto& target : targets) {
        if (target.active) {
            float dx = static_cast<float>(mouseX) - target.x;
            float dy = static_cast<float>(mouseY) - target.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= target.radius) {
                target.active = false;
                score++;
                hit = true;
                
                double timeSinceLastHit = currentTime - lastHitTime;
                totalHitTime += timeSinceLastHit;
                hitCount++;
                lastHitTime = currentTime;
                
                std::cout << "\nHIT! Pogodaka: " << score << std::endl;
                break;
            }
        }
    }
}

bool AimTrainer::shouldExit() const {
    return exitRequested;
}

bool AimTrainer::isPointInRect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}
