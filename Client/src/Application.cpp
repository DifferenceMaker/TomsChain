#define WIN32_LEAN_AND_MEAN // Prevent inclusion of winsock.h in Windows headers

#include <GL/glew.h> // OpenGL Extensions (This has to be added first)
#include <GLFW/glfw3.h> // creating window using GLFW

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

/* All Render components are imported separately */

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "ClientModule.h"




void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(void)
{

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //0.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);

    // Set the window size
    const int windowWidth = 1300;
    const int windowHeight = 750;

    // Get the parent directory (project directory)
    std::filesystem::path projectDirectory = std::filesystem::current_path().parent_path();
    std::filesystem::path fontFileDirectory = projectDirectory / "Dependencies" / "Fonts" / "VensimSans-Regular.ttf";

    // Display the project directory
    std::cout << "font File directory " << fontFileDirectory << std::endl;

    // Calculate the desired window position (centered on the screen)
    int screenWidth, screenHeight;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), NULL, NULL, &screenWidth, &screenHeight);
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;

    // set the window 
    window = glfwCreateWindow(windowWidth, windowHeight, "TomsChain", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    glfwSetWindowPos(window, windowX, windowY);

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "Error with glew initialization \n";

    std::cout << glGetString(GL_VERSION) << "\n";

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    
    {
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        Renderer renderer;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows


        ImGuiStyle& style = ImGui::GetStyle();

       
        style.Colors[ImGuiCol_Text] = ImVec4(1,1,1, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        //style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0.3f);

        style.Colors[ImGuiCol_Button] = ImColor(255, 255, 255, 30);
        style.Colors[ImGuiCol_ButtonActive] = ImColor(255, 255, 255, 60);
        style.Colors[ImGuiCol_ButtonHovered] = ImColor(255, 255, 255, 45);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 3.f));
              

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // Font
        // Convert to const char* with double backslashes
        std::string fontFileDirectoryStr = fontFileDirectory.generic_string();
        std::string fontFileDirectoryCStr;
        for (char c : fontFileDirectoryStr) {
            if (c == '\\') {
                fontFileDirectoryCStr += "\\\\";
            }
            else {
                fontFileDirectoryCStr += c;
            }
        }

        // Display the const char* representation
        const char* fontFileDirectoryCStrChar = fontFileDirectoryCStr.c_str();


        ImFont* pFont = io.Fonts->AddFontFromFileTTF(fontFileDirectoryCStrChar, 23.0f);
        ImFont* pFontC = io.Fonts->AddFontFromFileTTF(fontFileDirectoryCStrChar, 50.0f);
        
        if (pFont->IsLoaded()) {
            ImGui::PushFont(pFont);
        }

        
        /*
            New hierarchy: blockchain has both - MainModule and LoginModule. only one background then.
            the problem: LoginModule takes in MainModule.

            login module:
             * create new wallet
             * choose cached wallet
             * add existing wallet
             * delete cached wallet
            start module takes you to the main module
            main module:
             * overview of your wallet(s)
             * send
             * blockchain
             * connectivity/network tab
        */

        std::unique_ptr<client::ClientModule> clientModule = std::make_unique<client::ClientModule>();
        
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {

            /* Poll for and process events */
            glfwPollEvents();
            double timeValue = glfwGetTime();

            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
            renderer.Clear();

            /* Start a new frame for ImGui */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            clientModule->OnRender();
            clientModule->OnUpdate(timeValue);
            clientModule->OnImGuiRender(pFontC);

            //ImGui::ShowDemoWindow();

            /* Render ImGui */
            ImGui::Render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

        }

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}