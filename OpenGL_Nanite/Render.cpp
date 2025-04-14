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
    clusterCount(0), numTextures(0){}

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
        case 4:
            modeText += "texture";
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
    if (renderMode == 2) {
        depthShader = new Shader((GLchar*)"DepthVertex.txt", (GLchar*)"DepthFrag.txt");
        hzbManager = new HZBManager();
        hzbManager->Initialize(this->screenWidth, this->screenHeight);
        InitDepthBuffer();
    }
}


void Renderer::InitDepthBuffer()
{

    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error: Framebuffer is not complete!" << std::endl;
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

void Renderer::InitBuffers() {
    //初始化着色器
    renderShader = new Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");

    computeShader = new Shader((GLchar*)"ComputeVisibility.comp");

    //初始化buffers
    //生成 VAO、VBO、EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    //VBO: 存储顶点位置
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), nullptr, GL_DYNAMIC_DRAW);

    // EBO: 存储索引
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices.size(), nullptr, GL_DYNAMIC_DRAW);

    // 顶点属性指针
    // 位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 纹理坐标
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);

    // 创建 SSBO 用于存储 GPUCluster 数据
    glGenBuffers(1, &ssboClusters);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);

    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 创建 SSBO 用于存储命令计数（原子计数器）
    glGenBuffers(1, &ssboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    unsigned int zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &zero, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &secondSsboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
    unsigned int two = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &two, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, secondSsboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //创建SSBO 存储三角形数量
    glGenBuffers(1, &ssboTriCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    unsigned int one = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &one, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboTriCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //初始化纹理数组
    glGenTextures(1, &textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

    // 设置纹理参数
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 4096, 4096, numTextures);

    // 设置纹理过滤和包装方式
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 绑定纹理数组
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);  // 解绑纹理数组
}

void Renderer::LoadTex(string dataName)
{
    for (int i = 0; i < numTextures; ++i) {
        string imagePath = dataName + "_" + to_string(i) + ".png";
        int width, height, channels;
        unsigned char* imageData = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);  // 加载图像，使用 RGBA 格式

        if (imageData) {
            //上传到纹理数组的第 i 层
            glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
            stbi_image_free(imageData);  // 释放图像数据
        }
        else {
            // 图像加载失败，使用白色纹理
            std::cout << "Failed to load texture: " << imagePath << ", using white texture." << std::endl;

            // 创建一个纯白色纹理
            vector<unsigned char> whiteData(4096 * 4096 * 4, 255);
            glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 4096, 4096, 1, GL_RGBA, GL_UNSIGNED_BYTE, whiteData.data());
        }

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0); 
    }
}

void Renderer::LoadClusters()
{
    InitBuffers();
    // 创建 SSBO 用于存储间接绘制命令数组
    glGenBuffers(1, &ssboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndirectCmd);

    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * clusterCount, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &secondSsboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboIndirectCmd);

    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * clusterCount, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, secondSsboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 上传顶点和索引数据到 GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_DYNAMIC_DRAW);


    // 上传 gpuClusters 数据到 ssboClusters
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuClusters.size() * sizeof(GPUCluster), gpuClusters.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboClusters); // 绑定到 binding=0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    gpuClusters.clear();
    vertices.clear();
    indices.clear();
}

void Renderer::GenerateClusters(vector<PackedCluster> clusters, int texID)
{
    numTextures++;
    for (auto c : clusters) {
        GPUCluster gc = {};
        gc.groupId = c.groupId;
        gc.lodBounds = c.lodBounds;
        gc.parentLodBounds = c.parentLodBounds;
        gc.lodError = c.lodError;
        gc.maxParentLodError = c.maxParentLodError;
        gc.mipLevel = c.mipLevel;
        gc.indexOffset = indices.size();
        gc.vertOffset = vertices.size();
        gc.indexCount = c.indexes.size();
        gc.visible = 0;
        gc.textureID = texID;
        //cout << vertices.size() << endl;
        vertices.insert(vertices.end(), c.verts.begin(), c.verts.end());
        //cout << indices.size() << endl;
        indices.insert(indices.end(), c.indexes.begin(), c.indexes.end());
        //cout << gpuClusters.size() << endl;
        gpuClusters.push_back(gc);
    }
    //cout << vertices.size() << " " << indices.size() <<  " " << gpuClusters.size() << endl;
  
}

void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cerr << "OpenGL Debug: " << message << std::endl;
}
void Renderer::Render()
{

    glm::mat4 transformMatrix = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    glm::mat4 viewMatrix = camera->GetViewMatrix();	
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->GetZoom()), (float)screenWidth / (float)screenHeight, 0.1f, 3000.0f);
    UpdateFPS();  

    u32 triCount = 0;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, nullptr);
    //第一次剔除
    if (firstFrame) {
        prevView = viewMatrix;
        prevProj = projectionMatrix;
    }
    else {
        prevView = currentView;
        prevProj = currentProj;
    }
    currentProj = projectionMatrix;
    currentView = viewMatrix;

    computeShader->Use();
    //摄像机视锥
    glm::vec4 planes[6];
    glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;
    for (int i = 0; i < 6; i++) {
        int sign = (i % 2 == 0) ? 1 : -1;
        planes[i] = glm::vec4(
            viewProjMatrix[0][3] + sign * viewProjMatrix[0][i / 2],
            viewProjMatrix[1][3] + sign * viewProjMatrix[1][i / 2],
            viewProjMatrix[2][3] + sign * viewProjMatrix[2][i / 2],
            viewProjMatrix[3][3] + sign * viewProjMatrix[3][i / 2]
        );
        planes[i] /= glm::length(glm::vec3(planes[i]));
    }

    glm::vec2 screenSize = { screenWidth, screenHeight };
    glUniform1i(glGetUniformLocation(computeShader->program, "isFirstFrame"), firstFrame ? 1 : 0);
    glUniform1i(glGetUniformLocation(computeShader->program, "occludeMode"), 1);
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevViewMat"), 1, GL_FALSE, glm::value_ptr(prevView));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevProjMat"), 1, GL_FALSE, glm::value_ptr(prevProj));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uViewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uProjMat"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform2fv(glGetUniformLocation(computeShader->program, "uScreenSize"), 1, glm::value_ptr(screenSize));
    glUniform4fv(glGetUniformLocation(computeShader->program, "frustumPlanes"), 6, glm::value_ptr(planes[0]));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hzbManager->GetPrevHZBTexture());

    glUniform1i(glGetUniformLocation(computeShader->program, "prevDepthTexture"), 0);

    // 重置命令计数器为 0
    u32 zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &zero);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 重置三角形计数
    u32 one = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &one);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glDispatchCompute(((clusterCount + 255) / 256), 1, 1);

    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    //粗略剔除后绘制
    renderShader->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
    glUniform1i(glGetUniformLocation(renderShader->program, "meshTextures"), 0);

    glUniform1ui(glGetUniformLocation(renderShader->program, "viewMode"), viewMode);

    int transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    int projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    unsigned int* firstCounterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
    u32 firstDrawCount = *firstCounterPtr;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
    glBindVertexArray(0);

    //初步剔除后的深度记录下来
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader->Use();
    glUniform1ui(glGetUniformLocation(depthShader->program, "viewMode"), viewMode);

    transformLocation = glGetUniformLocation(depthShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    projectionLocation = glGetUniformLocation(depthShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    viewLocation = glGetUniformLocation(depthShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
    hzbManager->UpdateCurrentHZB(depthTexture);

    //第二次剔除

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    computeShader->Use();
    glUniform1i(glGetUniformLocation(computeShader->program, "isFirstFrame"), 0);
    glUniform1i(glGetUniformLocation(computeShader->program, "occludeMode"), 2);
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevViewMat"), 1, GL_FALSE, glm::value_ptr(prevView));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uPrevProjMat"), 1, GL_FALSE, glm::value_ptr(prevProj));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uViewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uProjMat"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform2fv(glGetUniformLocation(computeShader->program, "uScreenSize"), 1, glm::value_ptr(screenSize));
    glUniform4fv(glGetUniformLocation(computeShader->program, "frustumPlanes"), 6, glm::value_ptr(planes[0]));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, hzbManager->GetCurrentHZBTexture());
    glUniform1i(glGetUniformLocation(computeShader->program, "curDepthTexture"), 1);

    // 重置命令计数器为 0
    u32 two = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &two);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glDispatchCompute(((clusterCount + 255) / 256), 1, 1);

    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    //补充渲染

    renderShader->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
    glUniform1i(glGetUniformLocation(renderShader->program, "meshTextures"), 0);

    glUniform1ui(glGetUniformLocation(renderShader->program, "viewMode"), viewMode);

    transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, secondSsboIndirectCmd);

    // 读取第二次补充的绘制命令数量
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, secondSsboCmdCounter);
    unsigned int* counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
    u32 secondDrawCount = *counterPtr;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //读取三角形数量
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
    triCount = *counterPtr;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, secondDrawCount, 0);
    glBindVertexArray(0);

    //最终深度绘制
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader->Use();
    glUniform1ui(glGetUniformLocation(depthShader->program, "viewMode"), viewMode);

    transformLocation = glGetUniformLocation(depthShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    projectionLocation = glGetUniformLocation(depthShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    viewLocation = glGetUniformLocation(depthShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, firstDrawCount, 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, secondSsboIndirectCmd);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, secondDrawCount, 0);
    hzbManager->UpdateCurrentHZB(depthTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderOverlay(this->fps, triCount, firstDrawCount + secondDrawCount, clusterCount, this->triCount, viewMode);
    // 交换缓冲
    hzbManager->SwapHZB();
    firstFrame = false;
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);    
    glFrontFace(GL_CCW);
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
