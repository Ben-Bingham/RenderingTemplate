#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_clip_space.hpp>

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <utility/OpenGl/Shader.h>
#include <utility/OpenGl/VertexAttributeObject.h>
#include <utility/OpenGl/Buffer.h>
#include <utility/OpenGl/GLDebug.h>
#include <utility/OpenGl/RenderTarget.h>

#include <utility/Camera.h>
#include <utility/TimeScope.h>

using namespace RenderingUtilities;

void glfwErrorCallback(int error, const char* description) {
    std::cout << "ERROR: GLFW has thrown an error: " << std::endl;
    std::cout << description << std::endl;
}

glm::ivec2 mousePosition{ };
void mouseMoveCallback(GLFWwindow* window, double x, double y) {
    mousePosition.x = static_cast<int>(x);
    mousePosition.y = static_cast<int>(y);
}

int main() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        std::cout << "ERROR: Failed to initialize GLFW." << std::endl;
    }

    glm::ivec2 windowSize{ 1600, 900 };
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    GLFWwindow* window = glfwCreateWindow(windowSize.x, windowSize.y, "Rutile", nullptr, nullptr);

    if (!window) {
        std::cout << "ERROR: Failed to create window." << std::endl;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR: Failed to initialize GLEW." << std::endl;
    }

    glfwSetCursorPosCallback(window, mouseMoveCallback);

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.FontGlobalScale = 2.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    glm::ivec2 defaultFramebufferSize{ 800, 600 };
    glm::ivec2 lastFrameViewportSize{ defaultFramebufferSize };

    RenderTarget rendererTarget{ defaultFramebufferSize };

    Shader solidShader{
        "assets\\shaders\\solid.vert",
        "assets\\shaders\\solid.frag"
    };

    Camera camera{ };

    VertexAttributeObject vao{ };
    VertexBufferObject vbo{ std::vector<float>{
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
            0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,     0.0f, 0.5f,
            0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,     1.0f, 0.0f
    } };

    ElementBufferObject ebo{ std::vector<unsigned int>{
        0, 1, 2
    } };

    glm::mat4 transform{ 1.0f };

    std::chrono::duration<double> frameTime{ };
    std::chrono::duration<double> renderTime{ };

    while (!glfwWindowShouldClose(window)) {
        TimeScope frameTimeScope{ &frameTime };

        glfwPollEvents();

        //glm::ivec2 mousePositionWRTViewport{ mousePosition.x - viewportOffset.x, lastFrameViewportSize.y - (viewportOffset.y - mousePosition.y) };

        //MoveCamera(camera, window, static_cast<float>(frameTime.count()), mousePositionWRTViewport, lastFrameViewportSize, mouseOverViewPort);

        {
            TimeScope renderingTimeScope{ &renderTime };

            rendererTarget.Bind();

            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            solidShader.Bind();
            solidShader.SetVec3("color", glm::vec3{ 1.0f, 0.0f, 0.0f });

            glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)rendererTarget.GetSize().x / (float)rendererTarget.GetSize().y, camera.nearPlane, camera.farPlane);
            glm::mat4 mvp = projection * camera.View() * transform;

            solidShader.SetMat4("mvp", mvp);

            vao.Bind();
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

            rendererTarget.Unbind();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        //ImGui::ShowMetricsWindow();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // Keep track of this so that we can make these changes after the imgui frame is finished
        size_t changedPointLightIndex{ 0 };
        bool pointLightChanged{ false };

        glm::ivec2 newViewportSize{ };

        { ImGui::Begin("Viewport");
            // Needs to be the first call after "Begin"
            newViewportSize = glm::ivec2{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };

            // Display the frame with the last frames viewport size (The same size it was rendered with)
            ImGui::Image((ImTextureID)rendererTarget.GetTexture().Get(), ImVec2{ (float)lastFrameViewportSize.x, (float)lastFrameViewportSize.y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

            //mouseOverViewPort = ImGui::IsItemHovered();

            //viewportOffset = glm::ivec2{ (int)ImGui::GetCursorPos().x, (int)ImGui::GetCursorPos().y }; // TODO

        } ImGui::End(); // Viewport

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        const ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* currentContextBackup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(currentContextBackup);
        }

        // After ImGui has rendered its frame, we resize the framebuffer if needed for next frame
        if (newViewportSize != lastFrameViewportSize) {
            rendererTarget.Resize(newViewportSize);
        }

        lastFrameViewportSize = newViewportSize;

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwTerminate();
}