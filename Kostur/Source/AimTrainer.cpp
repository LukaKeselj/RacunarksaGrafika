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
      textRenderer(nullptr), exitRequested(false), totalClicks(0)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    shaderProgram = createShader("Shaders/basic.vert", "Shaders/circle.frag");
    textShaderProgram = createShader("Shaders/text.vert", "Shaders/text.frag");
    textureShaderProgram = createShader("Shaders/texture.vert", "Shaders/texture.frag");
    freetypeShaderProgram = createShader("Shaders/freetype.vert", "Shaders/freetype.frag");
    
    textRenderer = new TextRenderer(freetypeShaderProgram, windowWidth, windowHeight);
    if (!textRenderer->loadFont("C:/Windows/Fonts/arial.ttf", 48)) {
        std::cout << "Warning: Failed to load Arial font" << std::endl;
    }
    
    studentInfoTexture = loadImageToTexture("Resources/indeks.png");
    
    initBuffers();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
    float boxWidth = 450;
    float boxHeight = 400;
    float boxX = (windowWidth - boxWidth) / 2;
    float boxY = (windowHeight - boxHeight) / 2;
    
    restartButton.x = boxX + 105;
    restartButton.y = boxY + 270;
    restartButton.width = 240;
    restartButton.height = 50;
    restartButton.isHovered = false;
    
    exitButton.x = boxX + 105;
    exitButton.y = boxY + 330;
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
    glDeleteProgram(freetypeShaderProgram);
    glDeleteTextures(1, &studentInfoTexture);
    if (textRenderer) delete textRenderer;
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
    spawnInterval = initialSpawnInterval;
    targetLifeTimeMultiplier = 1.0f;
    hitCount = 0;
    totalHitTime = 0.0;
    gameOverTime = 0.0;
    survivalTime = 0.0;
    avgHitSpeed = 0.0;
    totalClicks = 0;
    
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
        
        drawRect(10, 10, 700, 80, 0.0f, 0.0f, 0.0f);
        drawRect(12, 12, 696, 76, 0.2f, 0.2f, 0.2f);
        
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
        timeStr << std::setfill('0') << std::setw(2) << minutes << ":" 
                << std::setw(2) << seconds << ":" 
                << std::setw(2) << centiseconds;
        
        textRenderer->renderText(timeStr.str(), 130, 35, 0.6f, 1.0f, 1.0f, 1.0f);
        
        std::stringstream statsStr;
        statsStr << "Hits: " << score << "/" << totalClicks;
        textRenderer->renderText(statsStr.str(), 330, 35, 0.5f, 0.4f, 1.0f, 0.4f);
        
        double avgSpeed = 0.0;
        if (hitCount > 0) {
            avgSpeed = totalHitTime / hitCount;
        }
        
        std::stringstream speedStr;
        speedStr << "Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " s";
        textRenderer->renderText(speedStr.str(), 480, 35, 0.45f, 1.0f, 0.8f, 0.3f);
        
        double hitSpeed = 0.0;
        if (hitCount > 0) {
            hitSpeed = totalHitTime / hitCount;
        }
        
        float accuracy = 0.0f;
        if (totalClicks > 0) {
            accuracy = (static_cast<float>(score) / static_cast<float>(totalClicks)) * 100.0f;
        }
        
        std::cout << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ":" << (centiseconds < 10 ? "0" : "") << centiseconds
                  << " | Zivoti: " << lives << "/" << maxLives 
                  << " | Pogodaka: " << score << "/" << totalClicks << " (" << std::fixed << std::setprecision(1) << accuracy << "%)"
                  << " | Avg Speed: " << hitSpeed << "s         \r" << std::flush;
    } else {
        float boxWidth = 450;
        float boxHeight = 400;
        float boxX = (windowWidth - boxWidth) / 2;
        float boxY = (windowHeight - boxHeight) / 2;
        
        drawRect(boxX, boxY, boxWidth, boxHeight, 0.0f, 0.0f, 0.0f);
        drawRect(boxX + 2, boxY + 2, boxWidth - 4, boxHeight - 4, 0.3f, 0.3f, 0.3f);
        
        std::string gameOverText = "GAME OVER";
        float gameOverWidth = textRenderer->getTextWidth(gameOverText, 1.0f);
        float gameOverX = boxX + (boxWidth - gameOverWidth) / 2;
        textRenderer->renderText(gameOverText, gameOverX, boxY + 50, 1.0f, 1.0f, 0.2f, 0.2f);
        
        int survivalMinutes = static_cast<int>(survivalTime) / 60;
        int survivalSeconds = static_cast<int>(survivalTime) % 60;
        std::stringstream timeText;
        timeText << "Time: " << survivalMinutes << ":" << std::setfill('0') << std::setw(2) << survivalSeconds;
        
        float timeWidth = textRenderer->getTextWidth(timeText.str(), 0.5f);
        float timeX = boxX + (boxWidth - timeWidth) / 2;
        textRenderer->renderText(timeText.str(), timeX, boxY + 110, 0.5f, 0.8f, 0.8f, 1.0f);
        
        float accuracy = 0.0f;
        if (totalClicks > 0) {
            accuracy = (static_cast<float>(score) / static_cast<float>(totalClicks)) * 100.0f;
        }
        
        std::stringstream accuracyText;
        accuracyText << "Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%";
        float accuracyWidth = textRenderer->getTextWidth(accuracyText.str(), 0.5f);
        float accuracyX = boxX + (boxWidth - accuracyWidth) / 2;
        textRenderer->renderText(accuracyText.str(), accuracyX, boxY + 150, 0.5f, 0.4f, 1.0f, 0.4f);
        
        std::stringstream hitsText;
        hitsText << "Hits: " << score << " / " << totalClicks;
        float hitsWidth = textRenderer->getTextWidth(hitsText.str(), 0.45f);
        float hitsX = boxX + (boxWidth - hitsWidth) / 2;
        textRenderer->renderText(hitsText.str(), hitsX, boxY + 185, 0.45f, 0.9f, 0.9f, 0.9f);
        
        double avgSpeed = 0.0;
        if (hitCount > 0) {
            avgSpeed = totalHitTime / hitCount;
        }
        
        std::stringstream speedText;
        speedText << "Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " s";
        float speedWidth = textRenderer->getTextWidth(speedText.str(), 0.45f);
        float speedX = boxX + (boxWidth - speedWidth) / 2;
        textRenderer->renderText(speedText.str(), speedX, boxY + 220, 0.45f, 1.0f, 0.8f, 0.3f);
        
        drawRect(restartButton.x, restartButton.y, restartButton.width, restartButton.height, 0.2f, 0.8f, 0.2f);
        
        float restartTextWidth = textRenderer->getTextWidth("RESTART", 0.5f);
        float restartTextX = restartButton.x + (restartButton.width - restartTextWidth) / 2;
        textRenderer->renderText("RESTART", restartTextX, restartButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
        drawRect(exitButton.x, exitButton.y, exitButton.width, exitButton.height, 0.8f, 0.2f, 0.2f);
        
        float exitTextWidth = textRenderer->getTextWidth("EXIT", 0.5f);
        float exitTextX = exitButton.x + (exitButton.width - exitTextWidth) / 2;
        textRenderer->renderText("EXIT", exitTextX, exitButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
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
    
    totalClicks++;
    
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
