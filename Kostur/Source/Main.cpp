#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"
#include "../Header/AimTrainer.h"
#include "../Header/Crosshair.h"

AimTrainer* game = nullptr;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && game) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        game->handleMouseClick(mouseX, mouseY);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 800;
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Aim Trainer", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLFWcursor* crosshairCursor = createCrosshairCursor();
    if (crosshairCursor) {
        glfwSetCursor(window, crosshairCursor);
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

    game = new AimTrainer(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        glClear(GL_COLOR_BUFFER_BIT);

        game->update(deltaTime);
        game->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete game;
    
    if (crosshairCursor) {
        glfwDestroyCursor(crosshairCursor);
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}