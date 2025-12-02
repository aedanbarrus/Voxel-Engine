#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "SHADER.h"
#include "World.h"
#include "Physics.h"
#include "Particle_System.h"
#include "camera.h"
#include "TerrainGenerator.h"
#include "imgui.h"

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();

private:
    // Initialization
    bool initWindow();
    bool initOpenGL();
    bool initImGui();
    bool initShaders();
    bool initFramebuffers();
    bool initGeometry();
    bool initComputeShaders();
    bool initWorld();

    // Main loop functions
    void processInput();
    void processController();
    void update(float deltaTime);
    void render();
    void renderImGui();

    // ImGui panels
    void renderKernelEditor();
    void renderParticleEditor();

    // Rendering passes
    void renderSceneToFramebuffer();
    void renderFramebufferToScreen();

    // Input callbacks (static wrappers for GLFW)
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void joystickCallback(int jid, int event);

    // Utility functions
    void setupGridVisual(Grid& grid);
    void setupHeightmap(Heightmap& heightmap, int maxheight);
    std::vector<std::vector<float>> getHeightmap();

    // Window settings
    static constexpr unsigned int SCR_WIDTH = 1920;
    static constexpr unsigned int SCR_HEIGHT = 1080;

    // Window and context
    GLFWwindow* m_window;

    // Shaders
    std::unique_ptr<Shader> m_shadowShader;
    std::unique_ptr<Shader> m_chunkShader;
    std::unique_ptr<Shader> m_screenShader;
    std::unique_ptr<Shader> m_physicShader;
    std::unique_ptr<Shader> m_computeShader;
    std::unique_ptr<Shader> m_computeShader2;

    // World and physics
    std::unique_ptr<Earth> m_earth;
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<Grid> m_grid;
    std::unique_ptr<Heightmap> m_heightmap;
    std::vector<ParticleGenerator> m_particles;
    std::vector<ParticleGenerator> m_debugParticles;

    // Camera
    std::unique_ptr<Camera> m_camera;

    // Framebuffers
    unsigned int m_framebuffer;
    unsigned int m_textureColorbuffer;
    unsigned int m_depthTexture;
    unsigned int m_cloudTexture;

    // Geometry
    unsigned int m_quadVAO;
    unsigned int m_quadVBO;

    // Compute shader resources
    unsigned int m_ssbo;
    unsigned int m_maxDist;
    int m_sampleSize;
    int m_randomPoints;

    // Timing
    float m_deltaTime;
    float m_lastFrame;
    float m_pastTime[10];

    // Input state
    bool m_firstMouse;
    float m_lastX;
    float m_lastY;
    bool m_cursorOn;
    bool m_mouseLeftClicked;
    bool m_mlc;
    bool m_mrc;
    int m_jmlc;
    int m_jmrc;
    int m_addForceCount;

    // UI state
    float m_kernel[11];
    std::vector<int> m_placeBlock;
    bool m_lines;
    float m_scale;
    float m_threshhold;
    bool m_syncing;
    bool m_enableDebugParticles;

    // Static instance for callbacks
    static Application* s_instance;
};