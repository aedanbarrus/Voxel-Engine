#include "Application.h"

#include "Application.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Random.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
Application* Application::s_instance = nullptr;

Application::Application()
    : m_window(nullptr)
    , m_framebuffer(0)
    , m_textureColorbuffer(0)
    , m_depthTexture(0)
    , m_cloudTexture(0)
    , m_quadVAO(0)
    , m_quadVBO(0)
    , m_ssbo(0)
    , m_maxDist(0)
    , m_sampleSize(200)
    , m_randomPoints(1)
    , m_deltaTime(0.0f)
    , m_lastFrame(0.0f)
    , m_firstMouse(true)
    , m_lastX(SCR_WIDTH / 2.0f)
    , m_lastY(SCR_HEIGHT / 2.0f)
    , m_cursorOn(false)
    , m_mouseLeftClicked(false)
    , m_mlc(false)
    , m_mrc(false)
    , m_jmlc(0)
    , m_jmrc(0)
    , m_addForceCount(0)
    , m_lines(false)
    , m_scale(1.0f)
    , m_threshhold(0.0f)
    , m_syncing(false)
    , m_enableDebugParticles(true)
	, m_shutdown(false)
{
    s_instance = this;
    m_placeBlock = { 0, 0, 0 };
    for (int i = 0; i < 11; i++) m_kernel[i] = 0.0f;
    m_kernel[4] = 1.0f;
    for (int i = 0; i < 10; i++) m_pastTime[i] = 0.0f;
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    if (!initWindow()) return false;
    if (!initOpenGL()) return false;
    if (!initImGui()) return false;
    if (!initShaders()) return false;
    if (!initFramebuffers()) return false;
    if (!initGeometry()) return false;
    if (!initComputeShaders()) return false;
    if (!initWorld()) return false;

    std::cout << "You unlocked the achievement: Run the Program\n";
    return true;
}

bool Application::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Final Project", NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetJoystickCallback(joystickCallback);

    return true;
}

bool Application::initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    return true;
}

bool Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig config;
    config.SizePixels = 18.0f;
    io.Fonts->Clear();
    io.Fonts->AddFontDefault(&config);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    return true;
}

bool Application::initShaders() {
    m_shadowShader = std::make_unique<Shader>("shadowShader.vert", "shadowShader.frag");
    m_chunkShader = std::make_unique<Shader>("cshader.vert", "cshader.frag");
    m_screenShader = std::make_unique<Shader>("fshader.vert", "fshader.frag");
    m_physicShader = std::make_unique<Shader>("physObjShader.vert", "physObjShader.frag");

    m_screenShader->use();
    m_screenShader->setInt("screenTexture", 0);

    return true;
}

bool Application::initFramebuffers() {
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glGenTextures(1, &m_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, m_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorbuffer, 0);

    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool Application::initGeometry() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    return true;
}

bool Application::initComputeShaders() {
    m_computeShader = std::make_unique<Shader>("noise.comp");
    m_computeShader2 = std::make_unique<Shader>("noise2.comp");

    glGenTextures(1, &m_cloudTexture);
    glBindTexture(GL_TEXTURE_3D, m_cloudTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, m_sampleSize, m_sampleSize, m_sampleSize, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_computeShader->use();

    glGenBuffers(1, &m_ssbo);
    glGenBuffers(1, &m_maxDist);

    std::vector<glm::vec3> points(m_randomPoints * m_randomPoints * m_randomPoints);
    for (int x = 0; x < m_randomPoints; x++) {
        for (int y = 0; y < m_randomPoints; y++) {
            for (int z = 0; z < m_randomPoints; z++) {
                points[x + y * m_randomPoints + z * m_randomPoints * m_randomPoints] =
                    (glm::vec3(random(), random(), random()) + glm::vec3(x, y, z)) *
                    float(m_sampleSize) / float(m_randomPoints);
            }
        }
    }

    GLuint zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_maxDist);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_maxDist);

    m_computeShader->setInt("volumeSize", m_randomPoints);
    glBindImageTexture(0, m_cloudTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(m_sampleSize, m_sampleSize, m_sampleSize);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    m_computeShader2->use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_maxDist);
    glBindImageTexture(0, m_cloudTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(m_sampleSize, m_sampleSize, m_sampleSize);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return true;
}

bool Application::initWorld() {
    m_camera = std::make_unique<Camera>(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 90.0f, 0.01f, 1000.0f);
    m_earth = std::make_unique<Earth>();
    m_particleSystem = std::make_unique<ParticleSystem>();
    m_grid = std::make_unique<Grid>(3, 3);
    m_heightmap = std::make_unique<Heightmap>(3, 3);

    m_grid->grid[m_grid->height / 2][m_grid->width / 2] =
        new GridSquare(glm::ivec2(m_grid->height / 2, m_grid->width / 2), glm::ivec2(-1));

    setupGridVisual(*m_grid);

    PhysicObject myObject;
    myObject.rotationQuaternion = glm::quat(1, 0, 0, 0);
    myObject.pos = m_camera->pos + glm::vec3(0.5, 5.0, 0.5);
    myObject.rotationMat = glm::mat3_cast(myObject.rotationQuaternion);
    myObject.velocity = glm::vec3(0, 0, 0);
    myObject.rotationVelocity = glm::vec3(0, 0, 0);
    myObject.calculateDerivedData();

    Gravity myForce;
    myForce.gravity = glm::vec3(0, -0.5, 0);
    myObject.myForces.push_back(&myForce);

    m_earth->physicObjects.push_back(myObject);
    m_earth->updateMesh();

    while (m_earth->getVoxel(m_camera->pos.x, m_camera->pos.y, m_camera->pos.z) != 0)
        m_camera->pos.y++;

    return true;
}

void Application::shutdown() {
	if (m_shutdown) return;
	m_shutdown = true;
    m_earth.reset();
    m_particleSystem.reset();
    m_grid.reset();
    m_heightmap.reset();

    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);
    if (m_textureColorbuffer) glDeleteTextures(1, &m_textureColorbuffer);
    if (m_depthTexture) glDeleteTextures(1, &m_depthTexture);
    if (m_cloudTexture) glDeleteTextures(1, &m_cloudTexture);
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_maxDist) glDeleteBuffers(1, &m_maxDist);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        for (int i = 0; i < 9; i++) {
            m_pastTime[i] = m_pastTime[i + 1];
        }
        m_pastTime[9] = m_deltaTime;
        processInput();
        processController();
        update(m_deltaTime);
        render();
        renderImGui();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::update(float deltaTime) {
    m_camera->step(deltaTime);
    m_camera->sunDirection = glm::vec3(0, sin(glfwGetTime() / 10), cos(glfwGetTime() / 10));
    m_earth->step(deltaTime);
    CollisionData data = { {} };
    m_earth->getCollisions(data);
    for (const Contact& corner : data.contacts) {
        ParticleType particle;
        particle.pos = corner.contactPoint;
        particle.velocity = glm::vec3(0, 0, 0);
        particle.velocityVar = 0.5f * glm::vec3(0.1, 0.1, 0.1);
        particle.beginColor = glm::vec4(1, 0, 0, 1);
        particle.endColor = glm::vec4(1, 1, 0, 0);
        particle.colorVar = glm::vec4(0.3, 0.15, 0.15, 0);
        particle.acceleration = glm::vec3(0, 0, 0);
        particle.beginSize = 0.02f;
        particle.endSize = 0.001f;
        particle.sizeVar = 0.01f;
        particle.lifetime = 0.5f;
        m_earth->addParticle(particle);
    }

    for (ParticleGenerator& particle : m_particles) {
        m_earth->genParticle(particle, deltaTime);
    }


    if (m_enableDebugParticles) {
        for (ParticleGenerator& particle : m_debugParticles) {
            m_earth->genParticle(particle, deltaTime);
        }
    }

    if (!m_mouseLeftClicked) {
        glm::vec3 point = m_earth->physicObjects[0].calculateCameraCollision(
            m_camera->pos, m_camera->direction);
        if (point != m_camera->direction * -1.0f + m_camera->pos) {
            m_earth->physicObjects[0].addForceAtBodyPoint(
                m_camera->direction * 12.5f * float(m_addForceCount), point);
        }
        m_addForceCount = 0;
    }
}

void Application::render() {
    renderSceneToFramebuffer();

    renderFramebufferToScreen();
}

void Application::renderSceneToFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_lines) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    m_chunkShader->use();
    int rotMatLoc = glGetUniformLocation(m_chunkShader->ID, "rotMat");
    glUniformMatrix4fv(rotMatLoc, 1, GL_FALSE, glm::value_ptr(m_camera->returnMatrixTransform()));
    m_chunkShader->setVec3("lightColor", glm::vec3(1, 1, 1));
    m_chunkShader->setVec3("lightPos", glm::vec3(
        16 + 32 * cos(glfwGetTime() / 10),
        100,
        16 + 32 * sin(glfwGetTime() / 10)
    ));
    m_chunkShader->setVec3("viewPos", m_camera->pos);

    m_earth->draw(*m_chunkShader, m_camera->pos);

    if (m_lines) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::renderFramebufferToScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    m_screenShader->use();

    int kernelLoc = glGetUniformLocation(m_screenShader->ID, "myKernel");
    glUniform1fv(kernelLoc, 11, m_kernel);
    m_screenShader->setVec3("cameraViewDirection", m_camera->direction);
    m_screenShader->setVec3("cameraSunDirection", m_camera->sunDirection);
    m_screenShader->setVec3("cameraPos", m_camera->pos);
    m_screenShader->setFloat("time", glfwGetTime());
    m_screenShader->setFloat("scale", m_scale);
    m_screenShader->setFloat("threshhold", m_threshhold);
    m_screenShader->setMat4("transform", m_camera->returnMatrixTransform());

    m_screenShader->setInt("screenTexture", 0);
    m_screenShader->setInt("depthTexture", 1);
    m_screenShader->setInt("cloudTexture", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureColorbuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, m_cloudTexture);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Application::renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderKernelEditor();
    renderParticleEditor();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!s_instance) return;

    if (s_instance->m_cursorOn) return;

    if (s_instance->m_firstMouse) {
        s_instance->m_lastX = xpos;
        s_instance->m_lastY = ypos;
        s_instance->m_firstMouse = false;
    }

    float xoffset = xpos - s_instance->m_lastX;
    float yoffset = s_instance->m_lastY - ypos;
    s_instance->m_lastX = xpos;
    s_instance->m_lastY = ypos;

    float sensitivity = 0.025f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    float& pitch = s_instance->m_camera->pitch;
    float& yaw = s_instance->m_camera->yaw;
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -88.0f) pitch = -88.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    s_instance->m_camera->direction = glm::normalize(direction);
}

void Application::joystickCallback(int jid, int event) {
    if (event == GLFW_CONNECTED) {
        std::cout << "Joystick connected: " << jid << std::endl;
    }
    else if (event == GLFW_DISCONNECTED) {
        std::cout << "Joystick disconnected: " << jid << std::endl;
    }
}

void Application::processInput() {
    static bool pWasPressed = false;

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }

    glm::vec3 tempVel = glm::vec3(0.0f);
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        tempVel += glm::normalize(glm::vec3(m_camera->direction.x, 0, m_camera->direction.z));
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        tempVel += glm::normalize(glm::vec3(-m_camera->direction.x, 0, -m_camera->direction.z));
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        tempVel += glm::normalize(glm::vec3(m_camera->direction.z, 0, -m_camera->direction.x));
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        tempVel += glm::normalize(glm::vec3(-m_camera->direction.z, 0, m_camera->direction.x));
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        tempVel += glm::vec3(0, 1, 0);
    }
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        tempVel += glm::vec3(0, -1, 0);
    }

    if (glm::length(tempVel) != 0) {
        m_camera->velocity += glm::normalize(tempVel) * 0.075f * m_deltaTime;
    }

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        m_mouseLeftClicked = false;
    }

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (m_addForceCount < 128) m_addForceCount++;
        m_mouseLeftClicked = true;
        if (!m_mlc && !m_cursorOn) {
            m_earth->raycast(0);
            m_earth->updateMesh();
            m_mlc = true;
        }
    }
    else {
        m_mlc = false;
    }

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!m_mrc && !m_cursorOn) {
            m_earth->raycast(1);
            m_earth->updateMesh();
            m_mrc = true;
        }
    }
    else {
        m_mrc = false;
    }

    bool pIsPressed = glfwGetKey(m_window, GLFW_KEY_P) == GLFW_PRESS;
    if (pIsPressed && !pWasPressed) {
        m_cursorOn = !m_cursorOn;
        glfwSetInputMode(m_window, GLFW_CURSOR,
            m_cursorOn ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    pWasPressed = pIsPressed;
}

void Application::processController() {
    GLFWgamepadstate state;

    for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; jid++) {
        if (!glfwGetGamepadState(jid, &state)) continue;

        glm::vec3 tempVel = glm::vec3(0.0f);
        glm::vec2 leftStick = glm::vec2(
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]
        );
        glm::vec2 rightStick = glm::vec2(
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]
        );

        if (glm::length(leftStick) > 0.1f) {
            tempVel += glm::normalize(
                -leftStick.y * glm::vec3(m_camera->direction.x, 0, m_camera->direction.z) +
                leftStick.x * glm::vec3(-m_camera->direction.z, 0, m_camera->direction.x)
            );
        }

        if (glm::length(rightStick) > 0.1f) {
            m_camera->yaw += rightStick.x * 0.15f;
            m_camera->pitch += -rightStick.y * 0.15f;

            if (m_camera->pitch > 89.0f) m_camera->pitch = 89.0f;
            if (m_camera->pitch < -88.0f) m_camera->pitch = -88.0f;

            glm::vec3 direction;
            direction.x = cos(glm::radians(m_camera->yaw)) * cos(glm::radians(m_camera->pitch));
            direction.y = sin(glm::radians(m_camera->pitch));
            direction.z = sin(glm::radians(m_camera->yaw)) * cos(glm::radians(m_camera->pitch));
            m_camera->direction = glm::normalize(direction);
        }

        if (state.buttons[GLFW_GAMEPAD_BUTTON_A] ||
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] != -1) {
            tempVel += glm::vec3(0, 1, 0);
        }
        if (state.buttons[GLFW_GAMEPAD_BUTTON_Y] ||
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] != -1) {
            tempVel += glm::vec3(0, -1, 0);
        }

        if (glm::length(tempVel) != 0) {
            m_camera->velocity += glm::normalize(tempVel) * 0.075f * m_deltaTime;
        }

        if (!state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]) {
            m_mouseLeftClicked = false;
        }
        if (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]) {
            if (m_addForceCount < 128) m_addForceCount++;
            m_mouseLeftClicked = true;
            if (m_jmlc == 0 && !m_cursorOn) {
                m_earth->raycast(0);
                m_earth->updateMesh();
            }
            m_jmlc = 2;
        }
        else if (m_jmlc > 0) {
            m_jmlc -= 1;
        }

        if (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]) {
            if (m_jmrc == 0 && !m_cursorOn) {
                m_earth->raycast(1);
                m_earth->updateMesh();
            }
            m_jmrc = 2;
        }
        else if (m_jmrc > 0) {
            m_jmrc -= 1;
        }
    }
}

void Application::setupGridVisual(Grid& grid) {
    m_debugParticles.clear();

    for (int i = 0; i < grid.height; i++) {
        for (int j = 0; j < grid.width; j++) {
            if (grid.grid[i][j] != nullptr) {
                m_earth->setVoxel(i, grid.grid[i][j]->count, j, 1, false);
                if (grid.grid[i][j]->count != 0) {
                    m_earth->fill(i, 0, j, i, grid.grid[i][j]->count - 1, j, 0);
                }

                if (grid.grid[i][j]->cpos == glm::ivec2(-1)) continue;

                ParticleGenerator gen;
                gen.numberPerCycle = 1;
                gen.timeBetweenParticles = 0.1f;

                ParticleType type;
                type.pos = glm::vec3(i + 0.5f, grid.grid[i][j]->count + 1.f, j + 0.5f);
                glm::vec2 vel = glm::vec2(grid.grid[i][j]->cpos - grid.grid[i][j]->pos);
                glm::vec3 vel3 = glm::vec3(
                    vel.y * 0.5,
                    grid.grid[grid.grid[i][j]->cpos.y][grid.grid[i][j]->cpos.x]->count - grid.grid[i][j]->count,
                    vel.x * 0.5
                );
                type.lifetime = glm::length(vel3);
                type.velocity = glm::normalize(vel3);
                type.velocityVar = glm::vec3(0.1f, 0.1f, 0.01f) / type.lifetime;
                type.beginColor = glm::vec4(0, 1, 0, 1);
                type.endColor = glm::vec4(0, 0, 1, 0);
                type.beginSize = 0.2f;
                type.endSize = 0.1f;
                gen.type = type;

                m_debugParticles.push_back(gen);
            }
            else {
                m_earth->setVoxel(i, 0, j, 0, false);
            }
        }
    }
    m_earth->updateAllMeshes();
}

void Application::setupHeightmap(Heightmap& heightmap, int maxheight) {
    for (int h = 0; h < heightmap.data.size(); h++) {
        for (int w = 0; w < heightmap.data[0].size(); w++) {
            int height = int(heightmap.data[h][w] * maxheight);
            m_earth->fill(w, 0, h, w, height, h, 1);
            m_earth->fill(w, height + 1, h, w, maxheight, h, 0);
        }
    }
    m_earth->updateAllMeshes();
}

std::vector<std::vector<float>> Application::getHeightmap() {
    FILE* fp;
    errno_t err = fopen_s(&fp, "Heightmap.json", "r");
    if (!fp) {
        std::cerr << "Could not open file.\n";
        return {};
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.IsArray() || doc.Size() < 2) {
        std::cerr << "Invalid heightmap format.\n";
        return {};
    }

    const auto& array2D = doc[1];
    if (!array2D.IsArray()) {
        std::cerr << "Second element is not an array.\n";
        return {};
    }

    std::vector<std::vector<float>> data;
    data.reserve(array2D.GetArray().Size());

    for (const auto& row : array2D.GetArray()) {
        if (!row.IsArray()) continue;
        std::vector<float> rowData;
        rowData.reserve(row.GetArray().Size());
        for (const auto& item : row.GetArray()) {
            if (item.IsNumber()) {
                rowData.push_back(item.GetFloat());
            }
        }
        data.push_back(rowData);
    }

    return data;
}

void Application::renderKernelEditor() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(450, 400));
    ImGui::Begin("Kernel Editor");

    ImGui::Text("Edit 3x3 Kernel:");
    ImGui::SliderFloat3("Row 1", &m_kernel[0], -10.0, 10.0);
    ImGui::SliderFloat3("Row 2", &m_kernel[3], -10.0, 10.0);
    ImGui::SliderFloat3("Row 3", &m_kernel[6], -10.0, 10.0);
    ImGui::InputInt3("PlaceBlock", &m_placeBlock[0]);
    ImGui::SliderFloat("Threshhold", &m_threshhold, 0, 1);
    ImGui::SliderFloat("Scale", &m_scale, 0, 32);

    if (ImGui::Button("Update Shader")) {
        m_shadowShader->updateShader("shadowShader.vert", "shadowShader.frag");
        m_chunkShader->updateShader("cshader.vert", "cshader.frag");
        m_screenShader->updateShader("fshader.vert", "fshader.frag");
        m_physicShader->updateShader("physObjShader.vert", "physObjShader.frag");
    }

    if (ImGui::Button("Place Block")) {
        m_earth->setVoxel(m_placeBlock[0], m_placeBlock[1], m_placeBlock[2], 1, true);
        std::cout << m_earth->getVoxel(m_placeBlock[0], m_placeBlock[1], m_placeBlock[2]);
        m_earth->updateMesh();
    }

    if (ImGui::Button(m_lines ? "Disable Lines" : "Enable Lines")) {
        m_lines = !m_lines;
    }

    float sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += m_pastTime[i];
    }
    sum /= 10;
    ImGui::Text("%f FPS", 1 / sum);
    ImGui::Text("Pos: (%f, %f, %f)", m_camera->pos.x, m_camera->pos.y, m_camera->pos.z);

    static int amount = 1;
    ImGui::InputInt("Amount", &amount);

    if (ImGui::Button("Add")) {
        for (int i = 0; i < amount; i++) {
            m_grid->addSquare();
        }
        setupGridVisual(*m_grid);
    }

    if (ImGui::Button("Upscale")) {
        m_grid->upscale();
        setupGridVisual(*m_grid);
    }

    if (ImGui::Button("Calculate Counts")) {
        m_grid->calculateCounts();
        setupGridVisual(*m_grid);
    }

    if (ImGui::Button("Visualize Heightmap")) {
        setupHeightmap(*m_heightmap, 100);
    }

    if (ImGui::Button("Upscale Heightmap")) {
        m_heightmap->upscale();
        setupHeightmap(*m_heightmap, 100);
    }

    if (ImGui::Button("Import Heightmap")) {
        *m_heightmap = Heightmap(*m_grid);
        setupHeightmap(*m_heightmap, 100);
    }

    if (ImGui::Button("Blur Heightmap")) {
        m_heightmap->blur();
        setupHeightmap(*m_heightmap, 100);
    }

    ImGui::Checkbox("Enable Debug Particles", &m_enableDebugParticles);

    ImGui::End();
}

void Application::renderParticleEditor() {
    ImGui::SetNextWindowPos(ImVec2(450, 0));
    ImGui::SetNextWindowSize(ImVec2(450, 400));
    ImGui::Begin("Particle Editor");

    if (ImGui::Button("Add Particle")) {
        ParticleGenerator gen;
        gen.type.pos = m_camera->pos;
        gen.type.velocity = glm::vec3(0, 0, 0);
        gen.type.velocityVar = 0.5f * glm::vec3(0.1, 0.1, 0.1);
        gen.type.beginColor = glm::vec4(1, 0, 0, 1);
        gen.type.endColor = glm::vec4(1, 1, 0, 0);
        gen.type.colorVar = glm::vec4(0.3, 0.15, 0.15, 0);
        gen.type.acceleration = glm::vec3(0, 0, 0);
        gen.type.beginSize = 0.02;
        gen.type.endSize = 0.001;
        gen.type.sizeVar = 0.01;
        gen.type.lifetime = 0.5;
        gen.timePassed = 0;
        gen.numberPerCycle = 1;
        gen.timeBetweenParticles = 1;
        m_particles.push_back(gen);
    }

    for (size_t i = 0; i < m_particles.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::CollapsingHeader(("Particle " + std::to_string(i)).c_str())) {
            ImGui::InputFloat3("Position", glm::value_ptr(m_particles[i].type.pos));
            ImGui::InputFloat3("Velocity", glm::value_ptr(m_particles[i].type.velocity));
            ImGui::InputFloat3("Velocity Var", glm::value_ptr(m_particles[i].type.velocityVar));
            ImGui::ColorEdit4("Begin Color", glm::value_ptr(m_particles[i].type.beginColor));
            ImGui::ColorEdit4("End Color", glm::value_ptr(m_particles[i].type.endColor));
            ImGui::ColorEdit4("Color Var", glm::value_ptr(m_particles[i].type.colorVar));
            ImGui::InputFloat3("Acceleration", glm::value_ptr(m_particles[i].type.acceleration));
            ImGui::InputFloat("Begin Size", &m_particles[i].type.beginSize);
            ImGui::InputFloat("End Size", &m_particles[i].type.endSize);
            ImGui::InputFloat("Size Var", &m_particles[i].type.sizeVar);
            ImGui::InputFloat("Lifetime", &m_particles[i].type.lifetime);
            ImGui::InputFloat("Drag", &m_particles[i].type.drag);
            ImGui::InputFloat("Time Between Particles", &m_particles[i].timeBetweenParticles);
            ImGui::InputInt("Particles per Cycle", &m_particles[i].numberPerCycle);

            static int index = -1;
            ImGui::InputInt("Index", &index);
            if (index != -1 && index >= 0 && index < m_particles.size()) {
                std::cout << "\nSet Generator " << index << std::endl;
                ParticleGenerator& gen = m_particles[index];
                m_particles[i].type.generator = &gen;
                std::cout << m_particles[i].type.generator << ' ' << &m_particles[index] << std::endl;
                index = -1;
            }

            static float offset = 0.0f;
            ImGui::InputFloat("##offset", &offset);
            ImGui::SameLine();
            if (ImGui::Button("Add Offset")) {
                m_particles[i].timePassed += offset;
            }

            if (ImGui::Button("Duplicate")) {
                m_particles.insert(m_particles.begin() + i, m_particles[i]);
                ImGui::PopID();
                break;
            }
            ImGui::SameLine();
            if (ImGui::Button("Duplicate To Player")) {
                m_particles.insert(m_particles.begin() + i, m_particles[i]);
                m_particles[i + 1].type.pos = m_camera->pos;
                ImGui::PopID();
                break;
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                m_particles.erase(m_particles.begin() + i);
                ImGui::PopID();
                break;
            }
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Delete All")) {
        m_particles.clear();
    }

    static std::vector<bool> selectedGenerators;
    if (selectedGenerators.size() != m_particles.size()) {
        selectedGenerators = std::vector<bool>(m_particles.size(), false);
    }

    if (m_syncing) {
        ImGui::Text("Select Generators to Sync:");
        for (size_t i = 0; i < m_particles.size(); ++i) {
            std::string label = "Generator " + std::to_string(i);
            bool test = selectedGenerators[i];
            ImGui::Checkbox(label.c_str(), &test);
            selectedGenerators[i] = test;
        }
    }

    if (m_particles.size() != 0 && ImGui::Button(m_syncing ? "Confirm Sync" : "Sync")) {
        if (m_syncing) {
            for (size_t i = 0; i < m_particles.size(); i++) {
                if (selectedGenerators[i]) {
                    m_particles[i].timePassed = 0.0f;
                }
            }
        }
        m_syncing = !m_syncing;
        selectedGenerators = std::vector<bool>(m_particles.size(), false);
    }

    if (m_particles.size() != 0 && ImGui::Button("Export Particles"))
    {
        std::string textToCopy = "";
        for (int i = 0; i < m_particles.size(); i++)
        {
            textToCopy += std::to_string(m_particles[i].type.pos.x) + ",";
            textToCopy += std::to_string(m_particles[i].type.pos.y) + ",";
            textToCopy += std::to_string(m_particles[i].type.pos.z) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocity.x) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocity.y) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocity.z) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocityVar.x) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocityVar.y) + ",";
            textToCopy += std::to_string(m_particles[i].type.velocityVar.z) + ",";
            textToCopy += std::to_string(m_particles[i].type.beginColor.r) + ",";
            textToCopy += std::to_string(m_particles[i].type.beginColor.g) + ",";
            textToCopy += std::to_string(m_particles[i].type.beginColor.b) + ",";
            textToCopy += std::to_string(m_particles[i].type.beginColor.a) + ",";
            textToCopy += std::to_string(m_particles[i].type.endColor.r) + ",";
            textToCopy += std::to_string(m_particles[i].type.endColor.g) + ",";
            textToCopy += std::to_string(m_particles[i].type.endColor.b) + ",";
            textToCopy += std::to_string(m_particles[i].type.endColor.a) + ",";
            textToCopy += std::to_string(m_particles[i].type.colorVar.r) + ",";
            textToCopy += std::to_string(m_particles[i].type.colorVar.g) + ",";
            textToCopy += std::to_string(m_particles[i].type.colorVar.b) + ",";
            textToCopy += std::to_string(m_particles[i].type.colorVar.a) + ",";
            textToCopy += std::to_string(m_particles[i].type.acceleration.x) + ",";
            textToCopy += std::to_string(m_particles[i].type.acceleration.y) + ",";
            textToCopy += std::to_string(m_particles[i].type.acceleration.z) + ",";
            textToCopy += std::to_string(m_particles[i].type.beginSize) + ",";
            textToCopy += std::to_string(m_particles[i].type.endSize) + ",";
            textToCopy += std::to_string(m_particles[i].type.sizeVar) + ",";
            textToCopy += std::to_string(m_particles[i].type.lifetime) + ",";
            textToCopy += std::to_string(m_particles[i].type.drag) + ",";
            textToCopy += std::to_string(m_particles[i].timeBetweenParticles) + ",";
            textToCopy += std::to_string(m_particles[i].numberPerCycle) + "\n";
        }
        glfwSetClipboardString(m_window, textToCopy.c_str());
    }
    if (ImGui::Button("Import Particles"))
    {
        std::string text = glfwGetClipboardString(m_window);
        while (text.find("\n") != std::string::npos)
        {
            ParticleGenerator newGen;
            std::string line = text.substr(0, text.find("\n"));
            float x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            float y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            float z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.pos = glm::vec3(x, y, z);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.velocity = glm::vec3(x, y, z);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.velocityVar = glm::vec3(x, y, z);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            float w = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.beginColor = glm::vec4(x, y, z, w);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            w = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.endColor = glm::vec4(x, y, z, w);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            w = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.colorVar = glm::vec4(x, y, z, w);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            y = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            z = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.acceleration = glm::vec3(x, y, z);
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.beginSize = x;
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.endSize = x;
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.sizeVar = x;
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.lifetime = x;
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.type.drag = x;
            x = std::stof(line.substr(0, line.find(",")));
            line = line.substr(line.find(",") + 1, line.size());
            newGen.timeBetweenParticles = x;
            int n = std::stoi(line.substr(0, line.find(",")));
            newGen.numberPerCycle = n;
            newGen.timePassed = 0;

            m_particles.push_back(newGen);

            text = text.substr(text.find("\n") + 1, text.size());
        }
    }

    ImGui::End();
}