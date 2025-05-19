#include "System/Renderer.hpp"
#include "System/SKM_Loader.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinyfiledialogs.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

int display_w = 1280;
int display_h = 720;
SKM::SKMFile skmModel;
bool skmLoaded = false;
std::string loadedFilePath;
float toastTimer = 0.0f;
bool showToast = false;
std::string toastMessage;
bool geometryHidden = false;
bool wireframeShown = false;
//bool bonesShown = false;
bool gridShown = false;
bool uniformLighting = false;
static float cameraYaw = 0.f;
static float cameraPitch = 35.f;
static float cameraDistance = 1.f;
static float lightYaw = 135.f;
static float lightPitch = 60.f;
static float bgRed = .1f;
static float bgGreen = .2f;
static float bgBlue = .3f;
Renderer renderer;

#pragma region Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_KP_1:
                if (mods & GLFW_MOD_CONTROL)
                {
                    cameraPitch = 0.f;
                    cameraYaw = 180.f;
                }
                else
                {
                    cameraPitch = 0.f;
                    cameraYaw = 0.f;
                }
                break;
            case GLFW_KEY_KP_3:
                if (mods & GLFW_MOD_CONTROL)
                {
                    cameraPitch = 0.f;
                    cameraYaw = -90.f;
                }
                else
                {
                    cameraPitch = 0.f;
                    cameraYaw = 90.f;
                }
                break;
            case GLFW_KEY_G:
                if (mods & GLFW_MOD_CONTROL)
                {
                    gridShown = !gridShown;
                }
                break;
            default:
                break;
        }
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
            case GLFW_KEY_KP_2:
                if (cameraPitch - 15.f > -60.f)
                {
                    cameraPitch -= 15.f;
                }
                else
                {
                    cameraPitch = -60.f;
                }
                break;
            case GLFW_KEY_KP_4:
                if (cameraYaw - 15.f >= - 180.f)
                {
                    cameraYaw -= 15.f;
                }
                else
                {
                    cameraYaw = cameraYaw + 345.f;
                }
                break;
            case GLFW_KEY_KP_6:
                if (cameraYaw + 15.f <= 180.f)
                {
                    cameraYaw += 15.f;
                }
                else
                {
                    cameraYaw = cameraYaw - 345.f;
                }
                break;
            case GLFW_KEY_KP_8:
                if (cameraPitch + 15.f < 60.f)
                {
                    cameraPitch += 15.f;
                }
                else
                {
                    cameraPitch = 60.f;
                }
                break;
            default:
                break;
        }
    }
}
#pragma endregion

int main()
{
#if defined(NDEBUG) 
#ifdef _WIN32
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL)
        ShowWindow(hwnd, SW_HIDE);
#endif
#endif
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(display_w, display_h, "ToEE Model Viewer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    renderer.initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

#pragma region MenuOptionBools
        // file
        bool openClicked = false;
        bool reloadClicked = false;
        bool saveClicked = false;
        bool saveAsClicked = false;
        bool closeClicked = false;
        bool exitClicked = false;
        bool openShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false);
        bool reloadShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false);
        bool saveShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false);
        bool saveAsShortcut = io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_O, false);
        bool closeShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false) && skmLoaded;
        bool exitShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q, false);
        // options
        bool wireframeClicked = false;
        bool wireframeShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false);
        bool hideGeometryClicked = false;
        bool hideGeometryShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_H, false);
        //bool displayBonesClicked = false;
        //bool displayBonesShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B, false);
#pragma endregion

#pragma region MenuBar
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                openClicked = ImGui::MenuItem("Open SKM...", "Ctrl+O");
                reloadClicked = ImGui::MenuItem("Reload SKM", "Ctrl+R", false, skmLoaded);
                //saveClicked = ImGui::MenuItem("Save SKM", "Ctrl+S");
                //saveAsClicked = ImGui::MenuItem("Save SKM as..", "Ctrl+Shift+S");
                closeClicked = ImGui::MenuItem("Close SKM", "Ctrl+W", false, skmLoaded);
                exitClicked = ImGui::MenuItem("Exit", "Ctrl+Q");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options"))
            {
                wireframeClicked = ImGui::MenuItem("Show wireframe", "Ctrl+F", wireframeShown);
                hideGeometryClicked = ImGui::MenuItem("Hide geometry", "Ctrl+H", geometryHidden);
                //displayBonesClicked = ImGui::MenuItem("Render bones", "Ctrl+B", bonesShown);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
#pragma endregion

#pragma region MenuBar_functionality
        // file
        if (openClicked || openShortcut)
        {
            const char* filter[] = { "*.SKM" };
            const char* filePath = tinyfd_openFileDialog("Open SKM File", "", 1, filter, "SKM files", 0);

            if (filePath)
            {
                if (skmModel.loadFromFile(filePath))
                {
                    renderer.uploadMesh(skmModel.toMesh());
                    loadedFilePath = filePath;
                    skmLoaded = true;
                }
            }
        }

        if (reloadClicked || reloadShortcut)
        {
            if (skmModel.loadFromFile(loadedFilePath))
            {
                skmLoaded = true;
                renderer.clearMesh();
                renderer.uploadMesh(skmModel.toMesh());
                toastMessage = "Reloaded SKM file";
                toastTimer = 3.0f;
                showToast = true;
            }
        }

        /*if (saveClicked || saveShortcut)
        {

        }

        if (saveAsClicked || saveAsShortcut)
        {

        }*/

        if (closeClicked || closeShortcut)
        {
            skmModel = SKM::SKMFile();
            loadedFilePath.clear();
            skmLoaded = false;
            renderer.clearMesh();
        }

        if (exitClicked || exitShortcut)
        {
            glfwSetWindowShouldClose(window, true);
        }
        // options
        if (hideGeometryClicked || hideGeometryShortcut)
        {
            geometryHidden = !geometryHidden;
        }

        if (wireframeClicked || wireframeShortcut)
        {
            wireframeShown = !wireframeShown;
            glPolygonMode(GL_FRONT_AND_BACK, wireframeShown ? GL_LINE : GL_FILL);
        }

        //if (displayBonesClicked || displayBonesShortcut)
        //{
        //    bonesShown = !bonesShown;
        //}
#pragma endregion

        /*
        ImGui::Begin("Debug Info");
        if (skmLoaded)
        {
        }
        else
        {
        }
        ImGui::End();
        */

        ImGuiViewport* viewport = ImGui::GetMainViewport();

#pragma region Status_Bar
        const float statusBarHeight = ImGui::GetFrameHeightWithSpacing();

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusBarHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusBarHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));

        ImGui::Begin("StatusBar", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );

        if (skmLoaded)
            ImGui::Text("Loaded: %s", loadedFilePath.c_str());
        else
            ImGui::Text("No SKM file loaded.");

        ImGui::End();
        ImGui::PopStyleVar(3);
#pragma endregion

#pragma region Side_panel
        const float panelWidth = 160.0f;
        const float menuBarHeight = ImGui::GetFrameHeight();

        float panelHeight = viewport->Size.y - menuBarHeight - statusBarHeight;
        float panelPosY = viewport->Pos.y + menuBarHeight;

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - panelWidth, panelPosY));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        ImGui::Begin("Model Info", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar
        );

        if (skmLoaded)
        {
            ImGui::Text("Bones: %d", (uint32_t)skmModel.bones.size());
            ImGui::Text("Vertices: %d", (uint32_t)skmModel.vertices.size());
            ImGui::Text("Faces: %d", (uint32_t)skmModel.faces.size());
            ImGui::Text("Materials: %d", (uint32_t)skmModel.materials.size());
            ImGui::Text("Animations: NYI");
            ImGui::Separator();
            ImGui::Checkbox("Show Grid (Ctrl+G)", &gridShown);
            ImGui::Text("Pitch");
            ImGui::SliderFloat("###Pitch", &cameraPitch, -60.f, 60.f);
            if (ImGui::Button("Set to 0"))
            {
                cameraPitch = 0.f;
            }
            ImGui::Text("Rotation");
            ImGui::SliderFloat("###Yaw", &cameraYaw, -180.f, 180.f);
            if (ImGui::Button("+90"))
            {
                cameraYaw + 90.f > 180.f ? cameraYaw -= 270.f : cameraYaw += 90.f;
            }
            ImGui::SameLine();
            if (ImGui::Button("-90"))
            {
                cameraYaw - 90.f < -180.f ? cameraYaw += 270.f : cameraYaw -= 90.f;
            }
            if (ImGui::Button("Reset Rotation"))
            {
                cameraYaw = 0.f;
                cameraPitch = 35.f;
            }
            if (ImGui::Button("Reset Zoom"))
            {
                cameraDistance = 1.f;
            }
            ImGui::Separator();
            ImGui::Checkbox("Uniform Lighting", &uniformLighting);
            ImGui::Text("Light Pitch");
            ImGui::SliderFloat("###LightPitch", &lightPitch, 0.f, 90.f);
            ImGui::Text("Light Rotation");
            ImGui::SliderFloat("###LightYaw", &lightYaw, 0.f, 360.f);
            if (ImGui::Button("Reset Light"))
            {
                lightYaw = 135.f;
                lightPitch = 60.f;
            }
            ImGui::Separator();
            ImGui::Text("Background Color");
            ImGui::SliderFloat("R", &bgRed, 0.f, 1.f);
            ImGui::SliderFloat("G", &bgGreen, 0.f, 1.f);
            ImGui::SliderFloat("B", &bgBlue, 0.f, 1.f);
            if (ImGui::Button("Reset Color"))
            {
                bgRed = .1f;
                bgGreen = .2f;
                bgBlue = .3f;
            }
            ImGui::Separator();
        }
        else
        {
            ImGui::Text("No SKM loaded.");
        }

        ImGui::End();
#pragma endregion

#pragma region Reload_Toast
        if (showToast && toastTimer > 0.0f)
        {
            toastTimer -= ImGui::GetIO().DeltaTime;
            if (toastTimer <= 0.0f)
            {
                showToast = false;
            }
            else
            {
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImVec2 toastPos = ImVec2(viewport->Pos.x + 20, viewport->Pos.y + 20);

                ImGui::SetNextWindowBgAlpha(0.8f);
                ImGui::SetNextWindowPos(toastPos, ImGuiCond_Always);
                ImGui::Begin("Toast", nullptr,
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoNav);

                ImGui::TextUnformatted(toastMessage.c_str());
                ImGui::End();
            }
        }
#pragma endregion

        glfwGetFramebufferSize(window, &display_w, &display_h);

#pragma region Camera
        cameraDistance -= io.MouseWheel * 0.1f;
        cameraDistance = glm::clamp(cameraDistance, 0.1f, 100.f);

        float aspectRatio = (float)display_w / (float)display_h;

        float yawRad = glm::radians(cameraYaw);
        float pitchRad = glm::radians(cameraPitch);

        float radius = 10.f;

        float x = radius * cos(pitchRad) * sin(yawRad);
        float y = radius * sin(pitchRad);
        float z = radius * cos(pitchRad) * cos(yawRad);

        glm::vec3 target = glm::vec3(0.f, .5f, 0.f);
        glm::vec3 cameraPos = target + glm::vec3(x, y, z);

        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 proj = glm::ortho(-1.f * aspectRatio * cameraDistance, 1.f * aspectRatio * cameraDistance, -1.f * cameraDistance, 1.f * cameraDistance, .1f, 10000.f);
#pragma endregion

        glViewport(0, 0, display_w, display_h);
        glClearColor(bgRed, bgGreen, bgBlue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Light_vector
        float lightYawRad = glm::radians(-lightYaw);
        float lightPitchRad = glm::radians(-lightPitch);

        glm::vec3 lightDir = glm::vec3(0.f, 0.f, 0.f);
        lightDir.x = radius * cos(lightPitchRad) * sin(lightYawRad);
        lightDir.y = radius * sin(lightPitchRad);
        lightDir.z = radius * cos(lightPitchRad) * cos(lightYawRad);
        lightDir = glm::normalize(lightDir);
#pragma endregion

        if (gridShown)
            renderer.renderGrid(view, proj);

        if (!geometryHidden)
            renderer.render(view, proj, lightDir, uniformLighting);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
