#include <glad/glad.h>
#include "SHADER.h"
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "World.h"
#include "Physics.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "Particle_System.h"
#include "camera.h"
#include <omp.h>
#define _CRT_SECURE_NO_WARNINGS

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void resetChunk(Chunk& myChunk);
inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v);
std::vector<std::vector<float>> getHeighmap();
// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
Camera myCamera(glm::vec3(0,0,0),glm::vec3(0,0,1),90.0f,0.01,1000);

bool firstMouse = true;

float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;
float kernel[11] = { 0,0,0,0,1,0,0,0,0,0,0 };
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float pastTime[] = { 0,0,0,0,0,0,0,0,0,0 };
bool cursorOn = false;
bool mouseLeftClicked = false;
int addForceCount = 0;
bool mlc = false;
bool mrc = false;
float scale = 1;
float threshhold = 0;
glm::vec3 spin = glm::vec3(0.0,0.0,0.0);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Final Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init("#version 460 core");
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    Shader shadowShader("shadowShader.vert", "shadowShader.frag");
    Shader chunkShader("cshader.vert", "cshader.frag");
    Shader screenShader("fshader.vert", "fshader.frag");
    Shader physicShader("physObjShader.vert", "physObjShader.frag");
    PhysicObject myObject = PhysicObject();

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3,
        7, 5, 4,
        7, 6, 5,
        8, 9, 11,
        9, 10, 11,
        15, 13, 12,
        15, 14, 13,
        16, 17, 19,
        17, 18, 19,
        23, 21, 20,
        23, 22, 21,
    };

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    float time;
    Chunk myChunk = Chunk();
    Earth myEarth = Earth();
    ParticleSystem myPartSystem = ParticleSystem();
    ParticleType particle = { glm::vec3(0,2,0),glm::vec3(0,0,0), 0.5f*glm::vec3(0.1,0.1,0.1),glm::vec4(1,0,0,1), glm::vec4(1,1,0,0), glm::vec4(0.3,0.15,0.15,0),glm::vec3(0,0,0),0.02, 0.001, 0.01,0.5};
    Camera::getCamera()->sunDirrection = glm::vec3(0, -1, 0);

    
    time = glfwGetTime();
    /*
    std::vector<std::vector<float>> heightmap = getHeighmap();
    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 1024; j++)
        {
            myEarth.fill(i-512, 0, j-512, i-512, 1 + int(heightmap[i][j] * 400), j-512, 1);
            myEarth.setVoxel(i - 512, 1 + int(heightmap[i][j] * 400), j - 512, 2, false);
        }
    }
    //*/
    std::cout << (glfwGetTime() - time) << '\n';
    time = glfwGetTime();
    myEarth.updateMesh();
    std::cout << (glfwGetTime() - time) << '\n';
    while (myEarth.getVoxel(Camera::getCamera()->pos.x, Camera::getCamera()->pos.y, Camera::getCamera()->pos.z) != 0)
        Camera::getCamera()->pos.y++;
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    std::cout << "You unlocked the achievment: Run the Program\n";

    Shader compute = Shader("noise.comp");
    Shader compute2 = Shader("noise2.comp");

    int sampleSize = 200;
    int randomPoints = 1;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, sampleSize, sampleSize, sampleSize, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    compute.use();

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    GLuint zero = 0;

    GLuint maxDist;
    glGenBuffers(1, &maxDist);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, maxDist);

    std::vector<glm::vec3> points;
    std::vector<float> distance;

    points.resize(randomPoints* randomPoints * randomPoints);
    distance.resize(sampleSize * sampleSize * sampleSize);

#define randomizePoints() for (int x = 0; x < randomPoints; x++) for (int y = 0; y < randomPoints; y++) for (int z = 0; z < randomPoints; z++) points[x+y*randomPoints+z*randomPoints*randomPoints] = (glm::vec3(random(), random(), random())+glm::vec3(x,y,z)) * float(sampleSize)/float(randomPoints);

    randomizePoints();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, maxDist);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, maxDist);
        
    compute.setInt("volumeSize",randomPoints);
    glBindImageTexture(0, textureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(sampleSize, sampleSize, sampleSize);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint* ptr = (GLuint*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    std::cout << glm::uintBitsToFloat(*ptr);
    compute2.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, maxDist);
    glBindImageTexture(0, textureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    glDispatchCompute(sampleSize, sampleSize, sampleSize);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // render loop
    // -----------
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 projection;
    projection = glm::perspective(
        glm::radians(90.0f),
        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
        0.01f,
        300.0f
    );
    glEnable(GL_CULL_FACE);
    //resetChunk(myEarth.quadrantI[0]);
    //std::cout << myEarth.quadrantI[0].worldPos << myEarth.quadrantII[0].worldPos << myEarth.quadrantIII[0].worldPos << myEarth.quadrantIV[0].worldPos;
    std::vector<int> placeBlock = { 0,0,0 };
    bool lines = false;
    myObject.rotationQuaternion.x = 0;
    myObject.rotationQuaternion.y = 0;
    myObject.rotationQuaternion.z = 0;
    myObject.rotationQuaternion.w = 1;

    myObject.pos = Camera::getCamera()->pos+glm::vec3(0.5,5.0,0.5);
    myObject.rotationMat = glm::mat3_cast(myObject.rotationQuaternion);
    myObject.velocity = glm::vec3(0, 0, 0);
    myObject.rotationVelocity = glm::vec3(0, 0, 0);

    myObject.calculateDerivedData();
    Gravity myForce;
    myForce.gravity = glm::vec3(0, -0.5, 0);
    myObject.myForces.push_back(&myForce);
    myEarth.physicObjects.push_back(myObject);
    //myEarth.physicObjects.push_back(myObject);
    //myObject.pos = Camera::getCamera()->pos + glm::vec3(0.5, 3.0, 0.5);

    Spring mySpring;
    mySpring.connectionPoint = glm::vec3(0.5, 0.5, 0.5);
    mySpring.springPos = glm::vec3(0, 5, 0);
    mySpring.springDis = 2;
    mySpring.springStrength = 3;

    myEarth.updateMesh();
    std::vector<ParticleGenerator> myParticles;
    while (!glfwWindowShouldClose(window))
    {
        CollisionData data = { {} };
        myEarth.getCollisions(data);
        for (const Contact& corner : data.contacts)
        {
            particle.pos = corner.contactPoint;
            myEarth.addParticle(particle);
        }
        for (ParticleGenerator& particle : myParticles)
        {
            myEarth.genParticle(particle,deltaTime);
        }
        spin = glm::vec3(0, 0, 0);
        ImGui::GetIO().FontGlobalScale = 1.0f;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(450, 300));
        ImGui::Begin("Kernel Editor");
        ImGui::Text("Edit 3x3 Kernel:");
        ImGui::SliderFloat3("Row 1", &kernel[0], -10.0, 10.0);
        ImGui::SliderFloat3("Row 2", &kernel[3], -10.0, 10.0);
        ImGui::SliderFloat3("Row 3", &kernel[6], -10.0, 10.0);
        ImGui::InputInt3("PlaceBlock", &placeBlock[0]);
        ImGui::SliderFloat("Threshhold", &threshhold, 0, 1);
        ImGui::SliderFloat("Scale", &scale, 0, 32);

        if (ImGui::Button("Update Shader")) {
            shadowShader.updateShader("shadowShader.vert", "shadowShader.frag");
            chunkShader.updateShader("cshader.vert", "cshader.frag");
            screenShader.updateShader("fshader.vert", "fshader.frag");
            physicShader.updateShader("physObjShader.vert", "physObjShader.frag");
        }
        if (ImGui::Button("Place Block")) {
            myEarth.setVoxel(placeBlock[0], placeBlock[1], placeBlock[2], 1, true);
            std::cout << myEarth.getVoxel(placeBlock[0], placeBlock[1], placeBlock[2]);
            myEarth.updateMesh();
        }
        if (ImGui::Button(lines==false?"Enable Lines":"Disable Lines")) {
            lines = !lines;
        }
        ImGui::SetNextWindowPos(ImVec2(450, 0));
        ImGui::Begin("Particle Editor");

        if (ImGui::Button("Add Particle")) {
            myParticles.push_back(ParticleGenerator{ ParticleType{ Camera::getCamera()->pos,glm::vec3(0,0,0), 0.5f * glm::vec3(0.1,0.1,0.1),glm::vec4(1,0,0,1), glm::vec4(1,1,0,0), glm::vec4(0.3,0.15,0.15,0),glm::vec3(0,0,0),0.02, 0.001, 0.01,0.5 },1,0,1});
        }

        for (size_t i = 0; i < myParticles.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::CollapsingHeader(("Particle " + std::to_string(i)).c_str())) {
                ImGui::InputFloat3("Position", glm::value_ptr(myParticles[i].type.pos));
                ImGui::InputFloat3("Velocity", glm::value_ptr(myParticles[i].type.velocity));
                ImGui::InputFloat3("Velocity Var", glm::value_ptr(myParticles[i].type.velocityVar));
                ImGui::ColorEdit4("Begin Color", glm::value_ptr(myParticles[i].type.beginColor));
                ImGui::ColorEdit4("End Color", glm::value_ptr(myParticles[i].type.endColor));
                ImGui::ColorEdit4("Color Var", glm::value_ptr(myParticles[i].type.colorVar));
                ImGui::InputFloat3("Acceleration", glm::value_ptr(myParticles[i].type.acceleration));
                ImGui::InputFloat("Begin Size", &myParticles[i].type.beginSize);
                ImGui::InputFloat("End Size", &myParticles[i].type.endSize);
                ImGui::InputFloat("Size Var", &myParticles[i].type.sizeVar);
                ImGui::InputFloat("Lifetime", &myParticles[i].type.lifetime);
                ImGui::InputFloat("Drag", &myParticles[i].type.drag);
                ImGui::InputFloat("Time Between Particles", &myParticles[i].timeBetweenParticles);
                ImGui::InputInt("Particles per Cycle", &myParticles[i].numberPerCycle);
                if (ImGui::Button("Duplicate")) {
                    myParticles.insert(myParticles.begin() + i, myParticles[i]);
                    ImGui::PopID();
                    break;
                }
                ImGui::SameLine();
                if (ImGui::Button("Duplicate To Player")) {
                    myParticles.insert(myParticles.begin() + i , myParticles[i]);
                    myParticles[i+1].type.pos = Camera::getCamera()->pos;
                    ImGui::PopID();
                    break;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    myParticles.erase(myParticles.begin() + i);
                    ImGui::PopID();
                    break;
                }

            }
            ImGui::PopID();
            if (i==myParticles.size()-1&&ImGui::Button("Delete All"))
            {
                myParticles.clear();
            }
        }

        ImGui::End();

        if (!mouseLeftClicked)
        {
            glm::vec3 point = myEarth.physicObjects[0].calculateCameraCollision(Camera::getCamera()->pos, Camera::getCamera()->direction);
            if(point!= Camera::getCamera()->direction *-1.0f+ Camera::getCamera()->pos)
                myEarth.physicObjects[0].addForceAtBodyPoint(Camera::getCamera()->direction * 12.5f * float(addForceCount), point);
            addForceCount = 0;
        }
        if (lines == true)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (int i = 0; i < 9; i++)
        {
            pastTime[i] = pastTime[i + 1];
        }
        pastTime[9] = deltaTime;
        float sum = 0;
        for (int i = 0; i < 10; i++)
        {
            sum += pastTime[i];
        }
        sum /= 10;
        ImGui::Text("%f FPS", 1 / sum);
        ImGui::Text("Pos: (%f, %f, %f)", Camera::getCamera()->pos.x, Camera::getCamera()->pos.y, Camera::getCamera()->pos.z);
        ImGui::End();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (mlc == false && !cursorOn)
            {
                myEarth.raycast(0);
                myEarth.updateMesh();
                mlc = true;
            }
        }
        else
            mlc = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            if (mrc == false && !cursorOn)
            {
                myEarth.raycast(1);
                myEarth.updateMesh();
                mrc = true;
            }
        }
        else
            mrc = false;
        processInput(window);
        Camera::getCamera()->step(deltaTime);
        myEarth.step(deltaTime);
        Camera::getCamera()->sunDirrection = glm::vec3(0, sin(glfwGetTime() / 10), cos(glfwGetTime() / 10));
        //glClearDepth(0.5f);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        chunkShader.use();
        int rotMatLoc = glGetUniformLocation(chunkShader.ID, "rotMat");
        glUniformMatrix4fv(rotMatLoc, 1, GL_FALSE, glm::value_ptr(Camera::getCamera()->returnMatrixTransform()));
        chunkShader.setVec3("lightColor", glm::vec3(1, 1, 1));
        chunkShader.setVec3("lightPos", glm::vec3(16 + 32 * cos(glfwGetTime() / 10), 100, 16 + 32 * sin(glfwGetTime() / 10)));
        chunkShader.setVec3("viewPos", Camera::getCamera()->pos);
        myEarth.draw(chunkShader, Camera::getCamera()->pos);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //framebuffer draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        int kernelLoc = glGetUniformLocation(screenShader.ID, "myKernel");
        if (lines == true)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glUniform1fv(kernelLoc, 11, kernel);
        screenShader.setVec3("cameraViewDirection", Camera::getCamera()->direction);
        screenShader.setVec3("cameraSunDirection", Camera::getCamera()->sunDirrection);
        screenShader.setVec3("cameraPos", Camera::getCamera()->pos);

        screenShader.setFloat("time", glfwGetTime());
        screenShader.use();
        screenShader.setInt("screenTexture", 0);
        screenShader.setInt("depthTexture", 1);
        screenShader.setInt("cloudTexture", 2);
        screenShader.setFloat("scale", scale);
        screenShader.setFloat("threshhold", threshhold);
        screenShader.setMat4("transform", Camera::getCamera()->returnMatrixTransform());
        
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void resetChunk(Chunk& myChunk)
{
    myChunk.reset();
    for (int i = 0; i < 32 * 32 * 32; i++)
    {
        if (float(std::rand()) / 0x7fff > 0.5)
            myChunk.setVoxel(i, int((std::rand()) / float(0x7fff) * 2 + 1));
    }
    myChunk.generateMesh();
}

void processInput(GLFWwindow* window)
{
    static bool pWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    glm::vec3 tempVel = glm::vec3(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(Camera::getCamera()->direction.x, 0, Camera::getCamera()->direction.z));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(-Camera::getCamera()->direction.x, 0, -Camera::getCamera()->direction.z));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(Camera::getCamera()->direction.z, 0, -Camera::getCamera()->direction.x));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(-Camera::getCamera()->direction.z, 0, Camera::getCamera()->direction.x));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(0, 1, 0));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        tempVel += glm::normalize(glm::vec3(0, -1, 0));
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        mouseLeftClicked = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (addForceCount < 128)
            addForceCount++;
        mouseLeftClicked = true;
    }
    bool pIsPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pIsPressed && !pWasPressed)
    {
        cursorOn = !cursorOn;
        if (cursorOn)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    }
    pWasPressed = pIsPressed;
    if (glm::length(tempVel) != 0)
        Camera::getCamera()->velocity += (glm::normalize(tempVel) * 0.075f * deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);


}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (cursorOn)
        return;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.025f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    float& pitch = Camera::getCamera()->pitch;
    float& yaw = Camera::getCamera()->yaw;
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -88.0f)
        pitch = -88.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    Camera::getCamera()->direction = glm::normalize(direction);
}

std::vector<std::vector<float>> getHeighmap() {
    FILE* fp;
    errno_t err = fopen_s(&fp, "Heightmap.json", "r");
    if (!fp) {
        std::cerr << "Could not open file.\n";
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.IsArray()) {
        std::cerr << "Root is not an array.\n";
    }
    const auto& value = doc[0];
    const auto& array2D = doc[1];
    if (!array2D.IsArray()) {
        std::cerr << "Second element is not an array.\n";
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
inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}