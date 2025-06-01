#include "System/Camera.hpp"
#include "System/Logger.hpp"
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

#pragma region some global shit that could probably be a part of int main()
bool animsLoaded = false;
bool boneAxesShown = false;
bool boneOctahedronsShown = true;
bool cameraAsLightSource = false;
bool geometryHidden = false;
bool gridShown = false;
bool renderBones = false;
bool showAnimEvents = false;
bool showToast = false;
bool showTPose = false;
bool skmLoaded = false;
bool uniformLighting = false;
bool wireframeShown = false;

float bgBlue = .3f;
float bgGreen = .2f;
float bgRed = .1f;
float boneScaleFactor = 1.f;
float lightPitch = 35.f;
float lightYaw = 35.f;
float toastTimer = 0.0f;

int display_w = 1280;
int display_h = 720;
int selectedIndex = 0;

std::string loadedFilePath;
std::string toastMessage;
std::vector<std::string> animationNames;

Camera camera;
Renderer renderer;
SKM::SKMFile skmModel;
#pragma endregion

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
                camera.setEulerAngles(mods & GLFW_MOD_CONTROL ? 180.f : 0.f, 0.f);
                break;
            case GLFW_KEY_KP_3:
                camera.setEulerAngles(mods & GLFW_MOD_CONTROL ? -90.f : 90.f, 0.f);
                break;
            case GLFW_KEY_KP_7:
                camera.setEulerAngles(0.f, mods & GLFW_MOD_CONTROL ? 89.999f : -89.999f);
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
                camera.adjustEulerAngles(0.f, -15.f);
                break;
            case GLFW_KEY_KP_4:
                camera.adjustEulerAngles(-15.f, 0.f);
                break;
            case GLFW_KEY_KP_6:
                camera.adjustEulerAngles(15.f, 0.f);
                break;
            case GLFW_KEY_KP_8:
                camera.adjustEulerAngles(0.f, 15.f);
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
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);

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
        float timeValue = static_cast<float>(glfwGetTime());

        glfwPollEvents();

        glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        static ImVec2 lastMousePos = io.MousePos;
        ImVec2 currentMousePos = io.MousePos;
        ImVec2 mouseDelta = ImVec2(currentMousePos.x - lastMousePos.x, currentMousePos.y - lastMousePos.y);
        lastMousePos = currentMousePos;

        bool shiftHeld = io.KeyShift;
        bool middleMouseHeld = io.MouseDown[2];

#pragma region MenuOptionBools
        // file
        bool openClicked = false;
        bool reloadClicked = false;
        bool closeClicked = false;
        bool exitClicked = false;
        bool openShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false);
        bool reloadShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false);
        bool closeShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false) && skmLoaded;
        bool exitShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Q, false);
        // options
        bool wireframeClicked = false;
        bool hideGeometryClicked = false;
        bool renderBonesClicked = false;
        bool centerOnModelClicked = false;
        bool wireframeShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false);
        bool hideGeometryShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_H, false);
        bool renderBonesShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B, false);
        bool centerOnModelShortcut = io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_C, false);
        // tools
        bool animEventClicked = false;
#pragma endregion

#pragma region MenuBar
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                openClicked = ImGui::MenuItem("Open SKM...", "Ctrl+O");
                reloadClicked = ImGui::MenuItem("Reload SKM", "Ctrl+R", false, skmLoaded);
                closeClicked = ImGui::MenuItem("Close SKM", "Ctrl+W", false, skmLoaded);
                exitClicked = ImGui::MenuItem("Exit", "Ctrl+Q");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options"))
            {
                wireframeClicked = ImGui::MenuItem("Show wireframe", "Ctrl+F", wireframeShown);
                hideGeometryClicked = ImGui::MenuItem("Hide geometry", "Ctrl+H", geometryHidden);
                renderBonesClicked = ImGui::MenuItem("Render bones", "Ctrl+B", renderBones);
                centerOnModelClicked = ImGui::MenuItem("Center on model", "Shift+C");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                animEventClicked = ImGui::MenuItem("Animation Events", nullptr, showAnimEvents);

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
            const char* temp = tinyfd_openFileDialog("Open SKM File", "", 1, filter, "*.SKM", 0);

            std::string filePath;

            if (temp)
            {
                filePath = temp;
                std::replace(filePath.begin(), filePath.end(), '\\', '/');
            }

            if (filePath.length())
            {
                if (skmModel.loaded)
                {
                    animsLoaded = false;
                    skmModel.clear();
                }

                if (skmModel.loadFromFile(filePath))
                {
                    if (SKM::isOnExceptionList(filePath))
                    {
                        toastMessage = "Bone mismatch between SKM and SKA files. Bind pose skipped.";
                        toastTimer = 10.0f;
                        showToast = true;
                    }

                    animsLoaded = skmModel.populateAnimNames(animationNames);
                    renderer.uploadMesh(skmModel.toMesh());
                    loadedFilePath = filePath;
                    skmLoaded = true;
                }
            }
        }

        if (reloadClicked || reloadShortcut)
        {
            if (skmModel.loaded)
            {
                animsLoaded = false;
                skmModel.clear();
            }

            if (skmModel.loadFromFile(loadedFilePath))
            {
                skmLoaded = true;
                animsLoaded = skmModel.populateAnimNames(animationNames);
                renderer.uploadMesh(skmModel.toMesh());
                toastMessage = "Reloaded SKM file";
                toastTimer = 3.0f;
                showToast = true;
            }
        }

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
        }

        if (renderBonesClicked || renderBonesShortcut)
        {
            renderBones = !renderBones;
        }

        if (centerOnModelClicked || centerOnModelShortcut)
        {
            camera.setTarget(renderer.getModelCenter());
        }
        // tools
        if (animEventClicked)
        {
            showAnimEvents = !showAnimEvents;
        }
#pragma endregion

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

#pragma region Side_panel_right
        const float panelWidth = 200.0f;
        const float menuBarHeight = ImGui::GetFrameHeight();

        float panelHeight = viewport->Size.y - menuBarHeight - statusBarHeight + 1.f;
        float panelPosY = viewport->Pos.y + menuBarHeight - 1.f;

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - panelWidth, panelPosY));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        ImGui::Begin("Model Info", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar
        );

        ImGui::Text("Bones: %d", (uint32_t)skmModel.bones.size());
        ImGui::Text("Vertices: %d", (uint32_t)skmModel.vertices.size());
        ImGui::Text("Faces: %d", (uint32_t)skmModel.faces.size());
        ImGui::Text("Materials: %d", (uint32_t)skmModel.materials.size());
        ImGui::Text("Animations: NYI");
        ImGui::Separator();
        ImGui::Checkbox("Show grid (Ctrl+G)", &gridShown);

#pragma region camera_stuff
        glm::vec2 angles = camera.getEulerAnglesDeg();
        float pitch = angles.y;
        float yaw = angles.x;

        ImGui::Text("Pitch");
        bool pitchChanged = ImGui::SliderFloat("###Pitch", &pitch, -89.999f, 89.999f);
        if (ImGui::Button("Set to 0"))
        {
            pitch = 0.f;
            pitchChanged = true;
        }
        ImGui::Text("Rotation");
        bool yawChanged = ImGui::SliderFloat("###Yaw", &yaw, -180.f, 180.f);
        if (ImGui::Button("+90"))
        {
            yaw = (yaw + 90.f > 180.f) ? yaw - 270.f : yaw + 90.f;
            yawChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-90"))
        {
            yaw = (yaw - 90.f < -180.f) ? yaw + 270.f : yaw - 90.f;
            yawChanged = true;
        }
        if (ImGui::Button("Reset rotation"))
        {
            camera.reset();
        }
        if (ImGui::Button("Reset zoom"))
        {
            camera.resetZoom();
        }
        if (ImGui::Button("Reset panning"))
        {
            camera.resetPanning();
        }
        if (pitchChanged || yawChanged)
        {
            camera.setEulerAngles(yaw, pitch);
        }
#pragma endregion
        ImGui::Separator();
        ImGui::Checkbox("Camera as light source", &cameraAsLightSource);
        ImGui::Checkbox("Uniform lighting", &uniformLighting);
        ImGui::Text("Light Pitch");
        ImGui::SliderFloat("###LightPitch", &lightPitch, 0.f, 90.f);
        ImGui::Text("Light Rotation");
        ImGui::SliderFloat("###LightYaw", &lightYaw, 0.f, 360.f);
        if (ImGui::Button("Reset Light"))
        {
            lightYaw = 35.f;
            lightPitch = 35.f;
        }
        ImGui::Separator();
        ImGui::Text("Background color");
        ImGui::SliderFloat("R", &bgRed, 0.f, 1.f);
        ImGui::SliderFloat("G", &bgGreen, 0.f, 1.f);
        ImGui::SliderFloat("B", &bgBlue, 0.f, 1.f);
        if (ImGui::Button("Reset color"))
        {
            bgRed = .1f;
            bgGreen = .2f;
            bgBlue = .3f;
        }
        ImGui::Separator();
        ImGui::Checkbox("Render bone axes", &boneAxesShown);
        ImGui::Checkbox("Render bone shapes", &boneOctahedronsShown);
        ImGui::Text("Bone Scale");
        ImGui::SliderFloat("###BoneScale", &boneScaleFactor, .5f, 4.f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::End();
#pragma endregion

#pragma region Animation_Panels
        const float animPanelHeight = 130.f;
        const float animPanelPosY = viewport->Size.y - animPanelHeight - statusBarHeight;
        const float animPickerWidth = 200.f;
        const float animTimelineWidth = viewport->Size.x - panelWidth - animPickerWidth + 2.f;

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, animPanelPosY));
        ImGui::SetNextWindowSize(ImVec2(animPickerWidth, animPanelHeight));
#pragma region Anim_picker
        ImGui::Begin("Animation Picker", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar
        );
        ImGui::Checkbox("T-pose", &showTPose);
        if (ImGui::BeginChild("AnimationsList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (animsLoaded)
            {
                for (size_t i = 0; i < animationNames.size(); ++i)
                {
                    bool isSelected = (selectedIndex == static_cast<int>(i));

                    if (ImGui::Selectable(animationNames[i].c_str(), isSelected))
                    {
                        selectedIndex = static_cast<int>(i);
                        // todo
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::End();
#pragma endregion

#pragma region Anim_timeline
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + animPickerWidth - 1.f, animPanelPosY));
        ImGui::SetNextWindowSize(ImVec2(animTimelineWidth, animPanelHeight));
        ImGui::Begin("Animation Timeline", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar
        );
        ImGui::End();
#pragma endregion
#pragma endregion

#pragma region AnimEvents
        if (showAnimEvents)
        {
            float animEventPosY = 18.f;
            float animEventWidth = viewport->Size.x - panelWidth + 1;
            float animEventHeight = 140.f;

            ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, animEventPosY));
            ImGui::SetNextWindowSize(ImVec2(animEventWidth, animEventHeight));
            ImGui::Begin("Animation Event Info", nullptr,
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove
            );
            if (ImGui::BeginChild("EventList", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (ImGui::BeginTable("EventsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX))
                {
                    ImGui::TableSetupColumn("Frame ID");
                    ImGui::TableSetupColumn("Event Type");
                    ImGui::TableSetupColumn("Action");
                    ImGui::TableHeadersRow();

                    for (const auto& it : skmModel.animation.animEventData)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%d", it.frameId);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(it.eventType);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(it.action);
                    }

                    ImGui::EndTable();
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }
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
                ImVec2 toastPos = ImVec2(viewport->Pos.x + 10, viewport->Pos.y + 30);

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

#pragma region Camera
        if (!io.WantCaptureMouse)
            camera.zoom(io.MouseWheel);

        float aspectRatio = (float)display_w / (float)display_h;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = glm::ortho(-1.f * aspectRatio * camera.getDistance(), 1.f * aspectRatio * camera.getDistance(), -1.f * camera.getDistance(), 1.f * camera.getDistance(), .1f, 10000.f);

#pragma region panning
        if (!io.WantCaptureMouse && middleMouseHeld)
        {
            if (shiftHeld)
            {
                camera.pan(glm::vec2(mouseDelta.x, mouseDelta.y), display_h);
            }
            else
            {
                camera.rotate(-mouseDelta.x * 0.005f, -mouseDelta.y * 0.005f);
            }
        }
#pragma endregion
#pragma endregion

        glViewport(0, 0, display_w, display_h);
        glClearColor(bgRed, bgGreen, bgBlue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Light_vector
        float lightYawRad = glm::radians(lightYaw);
        float lightPitchRad = glm::radians(lightPitch);

        glm::vec3 lightDir = glm::vec3(1.f);

        if (cameraAsLightSource)
            lightDir = camera.getPosition();
        else
        {
            lightDir.x = 100.f * cos(lightPitchRad) * sin(lightYawRad);
            lightDir.y = 100.f * sin(lightPitchRad);
            lightDir.z = 100.f * cos(lightPitchRad) * cos(lightYawRad);
        }

        lightDir = glm::normalize(-lightDir);
#pragma endregion

        if (gridShown)
            renderer.renderGrid(view, proj);

        if (!geometryHidden)
        {
            glPolygonMode(GL_FRONT_AND_BACK, wireframeShown ? GL_LINE : GL_FILL);
            renderer.render(view, proj, lightDir, camera.getPosition(), uniformLighting, showTPose, timeValue);
        }

        if (renderBones && skmLoaded)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            renderer.renderBones(view, proj, boneScaleFactor, boneAxesShown, boneOctahedronsShown, glm::normalize(-camera.getBoneLightPosition()), showTPose);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    renderer.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
