#include "Render.h"

// 定义间接绘制命令结构
struct DrawElementsIndirectCommand {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

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
    VAO(0), VBO(0), EBO(0),
    ssboClusters(0), ssboIndirectCmd(0), ssboCmdCounter(0),ssboTriCount(0),
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

void RenderOverlay(float fps, int triangleCount, int nowClusterCount, int allClusterCount) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Triangles: %d", triangleCount);
    ImGui::Text("Now Clusters: %d", nowClusterCount);
    ImGui::Text("All Clusters: %d", allClusterCount);
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
    // 初始化顶点/索引和 SSBO 缓冲区
    InitBuffers();

    // 初始化 compute shader
    InitComputeShader();

    // 初始化渲染 shader
    InitRenderShader();

    //初始化GUI
    InitImGui(this->window);
}

void Renderer::InitBuffers()
{
    // 生成 VAO、VBO、EBO（此处顶点数据和索引数据来源于所有 Cluster 的合并）
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // VBO: 存储顶点位置
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 这里先不上传数据，待后续 GenerateClusters() 后上传
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertices.size(), nullptr, GL_DYNAMIC_DRAW);

    // EBO: 存储索引
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices.size(), nullptr, GL_DYNAMIC_DRAW);

    // 顶点属性：位置，location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // 创建 SSBO 用于存储 GPUCluster 数据
    glGenBuffers(1, &ssboClusters);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);
    // 先分配足够空间（clusterCount 后续赋值）
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 创建 SSBO 用于存储间接绘制命令数组
    glGenBuffers(1, &ssboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndirectCmd);
    
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * clusterCount, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboIndirectCmd);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 创建 SSBO 用于存储命令计数（原子计数器）
    glGenBuffers(1, &ssboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    unsigned int zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &zero, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCmdCounter);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //创建SSBO 存储三角形数量
    glGenBuffers(1, &ssboTriCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    unsigned int one = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &one, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboTriCount);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::InitComputeShader()
{
    computeShader = new Shader((GLchar*)"ComputeVisibility.comp");
}

void Renderer::InitRenderShader()
{
    // 加载 vertex 和 fragment shader
    renderShader = new Shader((GLchar*)"VertexShader.txt", (GLchar*)"FragmentShader.txt");
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
}

void Renderer::SetClusterCount(u32 clusterCount)
{
    this->clusterCount = clusterCount;
}

void Renderer::GenerateClusters(vector<PackedCluster>& clusters)
{
    // 为示例生成两个 Cluster，实际项目中你需要生成或加载真实数据
    // 这里假设每个 Cluster 数据已经合并到一个大顶点、索引数组中
    vertices.clear();
    indices.clear();
    gpuClusters.clear();

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
        cout << vertices.size() << endl;
        vertices.insert(vertices.end(), c.verts.begin(), c.verts.end());
        cout << indices.size() << endl;
        indices.insert(indices.end(), c.indexes.begin(), c.indexes.end());
        cout << gpuClusters.size() << endl;
        gpuClusters.push_back(gc);
    }
    

    // 上传顶点和索引数据到 GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_DYNAMIC_DRAW);

    // 上传 gpuClusters 数据到 ssboClusters
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuClusters.size() * sizeof(GPUCluster), gpuClusters.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboClusters); // 绑定到 binding=0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}



void Renderer::Render()
{

    // 清除颜色缓冲区
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    UpdateFPS();
    glm::mat4 transformMatrix = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    glm::mat4 viewMatrix = camera->GetViewMatrix();	// 求得观察矩阵
    glm::mat4 projectionMatrix = glm::perspective(camera->GetZoom(), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);


    // 计算可见性：调用 compute shader 根据摄像机位置更新每个 Cluster 的 visible 标志，并同时写入间接绘制命令到 ssboIndirectCmd（绑定点 1）以及更新命令计数（绑定点 2）。
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

    glUniformMatrix4fv(glGetUniformLocation(computeShader->program, "uViewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform4fv(glGetUniformLocation(computeShader->program, "frustumPlanes"), 6, glm::value_ptr(planes[0]));

    // 重置命令计数器为 0
    unsigned int zero = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &zero);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 重置三角形计数
    unsigned int one = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &one);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 按 Cluster 数量调度 compute shader，每个 invocation 处理一个 Cluster
    glDispatchCompute(clusterCount, 1, 1);

    // 确保 compute shader 写入的 SSBO 数据在后续绘制前可见
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    //使用间接绘制命令绘制所有可见的 Cluster
    renderShader->Use();
    // 传入变换矩阵和模式

    glUniform1ui(glGetUniformLocation(renderShader->program, "viewMode"), viewMode);

    int transformLocation = glGetUniformLocation(renderShader->program, "transformMatrix");
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));

    int projectionLocation = glGetUniformLocation(renderShader->program, "projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int viewLocation = glGetUniformLocation(renderShader->program, "viewMatrix");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(VAO);
    // 绑定间接绘制命令 SSBO 到默认缓冲区（需要将其绑定为 GL_DRAW_INDIRECT_BUFFER）
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ssboIndirectCmd);

    // 从 ssboCmdCounter 中读取实际绘制命令数量
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCmdCounter);
    unsigned int* counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
    unsigned int drawCount = *counterPtr;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //从 ssboTriCount读取三角形数量
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriCount);
    counterPtr = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), GL_MAP_READ_BIT);
    unsigned int triCount = *counterPtr;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    RenderOverlay(this->fps, triCount, drawCount, clusterCount);
    // 调用间接绘制，参数为命令个数
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, drawCount, 0);


    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboClusters);
    u32* data = (u32*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    for (int i = 0; i < clusterCount; ++i) {
        std::cout << "data[" << i << "] = " << data[i] << std::endl;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindVertexArray(0);*/
}

void Renderer::Cleanup()
{
    // 删除所有 OpenGL 对象
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &ssboClusters);
    glDeleteBuffers(1, &ssboIndirectCmd);
    glDeleteBuffers(1, &ssboCmdCounter);
    CleanupImGui();
}
