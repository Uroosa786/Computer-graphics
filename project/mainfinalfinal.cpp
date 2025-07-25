// Include standard headers
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Camera setup
glm::vec3 camPos(0.0f, 0.0f, 6.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);
float fov = 45.0f;
float cameraSpeed = 1.5f;
float cameraHorizontalAngle = 90.0f;
float cameraVerticalAngle = 0.0f;

// Vertex Shader Source
const char* getVertexShaderSource() {
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoords;\n"
        "uniform mat4 MVP;\n"
        "out vec3 vertexColor;\n"
        "out vec2 TexCoords;\n"
        "void main() {\n"
        "    vertexColor = aColor;\n"
        //"    TexCoords = vec2(1.0 - aTexCoords.s, aTexCoords.t);\n"
        "    TexCoords = aTexCoords;\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n"
        "}";
}

// Fragment Shader Source
const char* getFragmentShaderSource() {
    return
        "#version 330 core\n"
        "in vec3 vertexColor;\n"
        "in vec2 TexCoords;\n"
        "uniform vec3 overrideColor;\n"
        "uniform sampler2D texture1;\n"
        "uniform bool useTexture;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    vec3 color;\n"
        "    if(useTexture)\n"
        "        color = texture(texture1, TexCoords).rgb;\n"
        "    else\n"
        "        color = (overrideColor == vec3(-1.0)) ? vertexColor : overrideColor;\n"
        "    FragColor = vec4(color, 1.0f);\n"
        "}";
}

// Vertex structure
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoords;
};

// Shooting star structure
struct ShootingStar {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    std::vector<glm::vec3> trail;  // For storing tail positions
};

std::vector<ShootingStar> stars;
float starSpawnTimer = 0.0f;

// Sphere generator
std::vector<Vertex> generateSphere(int sectorCount, int stackCount, float radius = 0.5f) {
    std::vector<Vertex> vertices;
    float pi = glm::pi<float>();
    for (int i = 0; i < stackCount; ++i) {
        float stackAngle1 = pi / 2 - i * pi / stackCount;
        float stackAngle2 = pi / 2 - (i + 1) * pi / stackCount;
        float y1 = radius * sin(stackAngle1);
        float y2 = radius * sin(stackAngle2);
        float r1 = radius * cos(stackAngle1);
        float r2 = radius * cos(stackAngle2);
        float t1 = 1.0f - (float)i / stackCount;
        float t2 = 1.0f - (float)(i + 1) / stackCount;


        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * pi / sectorCount;
            float x1 = r1 * cos(sectorAngle);
            float z1 = r1 * sin(sectorAngle);
            float x2 = r2 * cos(sectorAngle);
            float z2 = r2 * sin(sectorAngle);
            float s = (float)j / sectorCount;

            vertices.push_back({ glm::vec3(x1, y1, z1), glm::vec3(1.0f), glm::vec2(s, t1) });
            vertices.push_back({ glm::vec3(x2, y2, z2), glm::vec3(1.0f), glm::vec2(s, t2) });
        }
    }
    return vertices;
}

// Shader compile and link
GLuint compileAndLinkShaders() {
    GLint success;
    char infoLog[512];

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vsrc = getVertexShaderSource();
    glShaderSource(vertexShader, 1, &vsrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex Shader failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsrc = getFragmentShaderSource();
    glShaderSource(fragmentShader, 1, &fsrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment Shader failed:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

// VAO/VBO
GLuint createVertexArrayObject(const std::vector<Vertex>& vertices) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    return VAO;
}

// Texture loading
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System with Shooting Stars", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glPointSize(3.0f);  // Shooting star size

    GLuint shader = compileAndLinkShaders();
    std::vector<Vertex> sphere = generateSphere(36, 18);
    GLuint VAO = createVertexArrayObject(sphere);
    int vertexCount = sphere.size();

    GLuint earthTexture = loadTexture("earth.jpg");
    GLuint moonTexture = loadTexture("moon.jpg");

    glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    glUseProgram(shader);
    GLuint mvpLoc = glGetUniformLocation(shader, "MVP");
    GLuint colorLoc = glGetUniformLocation(shader, "overrideColor");
    GLuint useTexLoc = glGetUniformLocation(shader, "useTexture");
    GLuint texLoc = glGetUniformLocation(shader, "texture1");
    glUniform1i(texLoc, 0);

    double lastX = 400, lastY = 300;
    glfwSetCursorPos(window, lastX, lastY);
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double dx = xpos - lastX;
        double dy = ypos - lastY;
        lastX = xpos;
        lastY = ypos;

        float cameraAngularSpeed = 6.0f;
        cameraHorizontalAngle -= dx * deltaTime * cameraAngularSpeed;
        cameraVerticalAngle -= dy * deltaTime * cameraAngularSpeed;
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        
        // Clamp horizontal angle 
        float minYaw = 60.0f;  // Adjust based on your scene
        float maxYaw = 120.0f; // Adjust based on your scene
        if (cameraHorizontalAngle < minYaw)
        {
            cameraHorizontalAngle = minYaw;
            glfwSetCursorPos(window, lastX, lastY); // Reset cursor so it doesn't "drift"
        }
        if (cameraHorizontalAngle > maxYaw)
        {
            cameraHorizontalAngle = maxYaw;
            glfwSetCursorPos(window, lastX, lastY); // Reset cursor so it doesn't "drift"
        }
        // Clamp vertical angle (pitch)
        float minPitch = -25.0f; // Adjust to prevent looking too far down
        float maxPitch = 25.0f;  // Adjust to prevent looking too far up

        if (cameraVerticalAngle < minPitch)
        {
            cameraVerticalAngle = minPitch;
            glfwSetCursorPos(window, lastX, lastY); // Reset cursor position to avoid drifting
        }
        if (cameraVerticalAngle > maxPitch)
        {
            cameraVerticalAngle = maxPitch;
            glfwSetCursorPos(window, lastX, lastY); // Reset cursor position to avoid drifting
        }

        glm::vec3 direction(
            cos(glm::radians(cameraVerticalAngle)) * cos(glm::radians(cameraHorizontalAngle)),
            sin(glm::radians(cameraVerticalAngle)),
            -cos(glm::radians(cameraVerticalAngle)) * sin(glm::radians(cameraHorizontalAngle))
        );
        glm::vec3 right = glm::normalize(glm::cross(direction, camUp));
        glm::vec3 up = glm::normalize(glm::cross(right, direction));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += direction * deltaTime * cameraSpeed; //forward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= direction * deltaTime * cameraSpeed; //back
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += right * deltaTime * cameraSpeed; //right
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * deltaTime * cameraSpeed; //left
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos += up * deltaTime * cameraSpeed;  //up
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camPos -= up * deltaTime * cameraSpeed; //down  

        glm::mat4 view = glm::lookAt(camPos, camPos + direction, up);
        glBindVertexArray(VAO);

        float t = currentTime;

        // Sun
        glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
        glm::mat4 sunMVP = projection * view * sunModel;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &sunMVP[0][0]);
        glUniform1i(useTexLoc, 0);
        glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

        // Earth
        glm::mat4 earthModel = glm::rotate(glm::mat4(1.0f), t * glm::radians(30.0f), glm::vec3(0, 1, 0));
        earthModel = glm::translate(earthModel, glm::vec3(2.3f, 0.0f, 0.0f));
        earthModel = glm::rotate(earthModel, t * glm::radians(100.0f), glm::vec3(0, 1, 0));
        glm::mat4 earthMVP = projection * view * earthModel;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &earthMVP[0][0]);
        glUniform1i(useTexLoc, 1);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

        // Moon
        glm::mat4 moonModel = glm::rotate(glm::mat4(1.0f), t * glm::radians(30.0f), glm::vec3(0, 1, 0));
        moonModel = glm::translate(moonModel, glm::vec3(2.3f, 0.0f, 0.0f));
        moonModel = glm::rotate(moonModel, t * glm::radians(100.0f), glm::vec3(0, 1, 0));
        moonModel = glm::translate(moonModel, glm::vec3(1.0f, 0.0f, 0.0f));
        moonModel = glm::scale(moonModel, glm::vec3(0.6f));
        glm::mat4 moonMVP = projection * view * moonModel;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &moonMVP[0][0]);
        glUniform1i(useTexLoc, 1);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

        // Spawn and update shooting stars
        starSpawnTimer += deltaTime;
        if (starSpawnTimer > 1.0f) {
            starSpawnTimer = 0.0f;
            ShootingStar s;
            s.position = glm::vec3(-5.0f + rand() % 10, 3.0f + rand() % 2, -5.0f);
            s.velocity = glm::vec3(2.5f, -3.0f, 0.0f);
            s.life = 2.0f;
            stars.push_back(s);
        }

        for (auto& s : stars) {
    // Add current position to trail
    s.trail.push_back(s.position);
    if (s.trail.size() > 10) // limit tail length
        s.trail.erase(s.trail.begin());
    s.position += s.velocity * deltaTime;
    s.life -= deltaTime;
}

        stars.erase(std::remove_if(stars.begin(), stars.end(), [](const ShootingStar& s) { return s.life <= 0.0f; }), stars.end());

        // Draw shooting stars
        for (const auto& s : stars) {
    // Draw tail
    float alphaStep = 1.0f / s.trail.size();
    float alpha = 0.0f;
    for (const auto& p : s.trail) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), p);
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
        glUniform1i(useTexLoc, 0);
        glUniform3f(colorLoc, alpha, alpha, alpha); // Fading brightness
        glDrawArrays(GL_POINTS, 0, 1);
        alpha += alphaStep;
    }

    // Draw bright head
    glm::mat4 model = glm::translate(glm::mat4(1.0f), s.position);
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);
    glUniform1i(useTexLoc, 0);
    glUniform3f(colorLoc, 1.5f, 1.5f, 1.5f);  // Slight glow
    glDrawArrays(GL_POINTS, 0, 1);
}


        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    glfwTerminate();
    return 0;
}
