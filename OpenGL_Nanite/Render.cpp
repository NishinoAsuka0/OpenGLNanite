#include "Render.h"

void Renderer::UpdateFPS() {
    float currentTime = glfwGetTime();
    frameCount++;

    if (currentTime - lastTime >= 1.0f) { // 每秒更新一次
        fps = frameCount / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
    }
}



Renderer::Renderer() :
    clusterCount(0){}

Renderer::~Renderer()
{
    Cleanup();
}

// 初始化 ImGui
void InitImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void RenderOverlay(float fps, int triangleCount, int nowClusterCount, int allClusterCount, int allTriCount, int viewMode) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(220, 135), ImGuiCond_Once);
    ImGui::Begin("Information");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Triangles: %d", triangleCount);
    ImGui::Text("All Triangles: %d", allTriCount);
    ImGui::Text("Now Clusters: %d", nowClusterCount);
    ImGui::Text("All Clusters: %d", allClusterCount);
    string modeText = "Now viewMode is ";
    switch (viewMode)
    {
        case 0:
            modeText += "triangleID";
            break;
        case 1:
            modeText += "clusterID";
            break;
        case 2:
            modeText += "clusterGroupID";
            break;
        case 3:
            modeText += "LODLevel";
            break;
    default:
        break;
    }
    ImGui::Text(modeText.c_str());
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CommonUI(float fps, int triangleCount) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(220, 135), ImGuiCond_Once);
    ImGui::Begin("Information");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Triangles: %d", triangleCount);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::Init()
{
    //初始化GUI
    InitImGui(this->window);
    depthShader = new Shader((GLchar*)"DepthVertex.txt", (GLchar*)"DepthFrag.txt");
    InitDepthBuffer();
}

void Renderer::InitDepthBuffer()
{

    for (int i = 0; i < 3; ++i) {
        glGenFramebuffers(1, &depthFBOs[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBOs[i]);

        glGenTextures(1, &depthTextures[i]);
        glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextures[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetCamera(Camera* camera)
{
    this->camera = camera;
}

void Renderer::SetScreenSize(GLFWwindow* window,int width, int height)
{
    this->window = window;
    screenWidth = width;
    screenHeight = height;
}

void Renderer::SetViewMode(u32 viewMode)
{
    this->viewMode = viewMode;
    for (auto& mesh : meshes) {
        mesh->SetViewMode(this->viewMode);
    }
}

void Renderer::SetRenderMode(u32 mode)
{
    this->renderMode = mode;
}

void Renderer::SetClusterCount(u32 clusterCount)
{
    this->clusterCount = clusterCount;
}

void Renderer::SetTriCount(u32 triCount)
{
    this->triCount = triCount;
}

void Renderer::AddMesh(Mesh* mesh)
{
    this->meshes.push_back(mesh);
}


void Renderer::Render()
{

    glm::mat4 transformMatrix = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    glm::mat4 viewMatrix = camera->GetViewMatrix();	// 求得观察矩阵
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->GetZoom()), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
    UpdateFPS();  

    u32 triCount = 0;
    u32 drawCount = 0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLuint usedDepthTexture = depthTextures[prevPrevDepthIndex];
    for (auto& mesh : meshes) {
        mesh->SetInfo(transformMatrix, model, viewMatrix, projectionMatrix);
        // 第一次剔除（深度剔除用preDepthTextrue）

        auto naniteMesh = static_cast<NaniteMesh*>(mesh);
        naniteMesh->ComputePass(usedDepthTexture, this->screenWidth, this->screenHeight, 1);
        // 初步绘制
        naniteMesh->Draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBOs[currentDepthIndex]);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto& mesh : meshes) {
        auto naniteMesh = static_cast<NaniteMesh*>(mesh);
        //第一步深度绘制
        depthShader->Use();
        naniteMesh->DrawDepth(depthShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLuint currentDepthTexture = depthTextures[currentDepthIndex];
    for (auto& mesh : meshes) {
        auto naniteMesh = static_cast<NaniteMesh*>(mesh);
        //第二次剔除
        naniteMesh->ComputePass(currentDepthTexture, this->screenWidth, this->screenHeight, 2);

        // 补充渲染 
        uvec2 data = naniteMesh->Draw();
        triCount += data.x;
        drawCount += data.y;
    }

    //最终深度绘制
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBOs[currentDepthIndex]);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto& mesh : meshes) {
        auto naniteMesh = static_cast<NaniteMesh*>(mesh);
        depthShader->Use();
        naniteMesh->DrawDepth(depthShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderOverlay(this->fps, triCount, drawCount, clusterCount, this->triCount, viewMode);


    // 交换缓冲
    int oldPrevPrev = prevPrevDepthIndex;
    prevPrevDepthIndex = prevDepthIndex;
    prevDepthIndex = currentDepthIndex;
    currentDepthIndex = oldPrevPrev;

}

void Renderer::CommonRender()
{
    glm::mat4 transformMatrix = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    glm::mat4 viewMatrix = camera->GetViewMatrix();	// 求得观察矩阵
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->GetZoom()), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    UpdateFPS();
    u32 triCount = 0;

    for (auto& mesh : meshes) {
        mesh->SetInfo(transformMatrix, model, viewMatrix, projectionMatrix);
        uvec2 data = mesh->Draw();
        triCount += data.x;
    }
    CommonUI(this->fps, triCount);

    glBindVertexArray(0);
}

void Renderer::Cleanup()
{
    CleanupImGui();
}
