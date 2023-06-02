#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <time.h>
#include <iostream>
#include <math.h>
#include <GL/freeglut.h>

const int GRID_SIZE = 10;
const int WINDOW_WIDTH = 1070;
const int WINDOW_HEIGHT = 935;
const float CELL_SPACING = 0.01f;
const float GAP_BOTTOM_TEXT = 0.2;
const float CELL_WIDTH =  (2.0f - CELL_SPACING) / GRID_SIZE;
const float CELL_HEIGHT = (2.0f - GAP_BOTTOM_TEXT) / GRID_SIZE;

float vertices[GRID_SIZE * GRID_SIZE * 4 * 3]; // 4 vertices per quad, 3 coordinates per vertex
float colors[GRID_SIZE * GRID_SIZE * 4 * 3];  // 4 vertices per quad, 3 components per color

GLuint vao, vbo, shader_programme;

int score = 0;
int moves = 5;

void randomColor(float* color)
{
    color[0] = static_cast<float>(rand() % 256) / 255.0f;
    color[1] = static_cast<float>(rand() % 256) / 255.0f;
    color[2] = static_cast<float>(rand() % 256) / 255.0f;
}

void createGrid()
{
    int vertexIndex = 0;
    int colorIndex = 0;

    for (int j = 0; j < GRID_SIZE; ++j)
    {
        for (int i = 0; i < GRID_SIZE; ++i)
        {
            float x = -1.0f + i * CELL_WIDTH + CELL_SPACING;
            float y = -1.0f + (GRID_SIZE - j - 1) * CELL_HEIGHT + GAP_BOTTOM_TEXT;

            // Vertices
            // Inferior esquerdo
            vertices[vertexIndex++] = x;
            vertices[vertexIndex++] = y;
            vertices[vertexIndex++] = 0.0f;

            // Inferior direito
            vertices[vertexIndex++] = x + CELL_WIDTH - CELL_SPACING;
            vertices[vertexIndex++] = y;
            vertices[vertexIndex++] = 0.0f;

            // Superior Direito
            vertices[vertexIndex++] = x + CELL_WIDTH - CELL_SPACING;
            vertices[vertexIndex++] = y + CELL_HEIGHT - CELL_SPACING;
            vertices[vertexIndex++] = 0.0f;

            // Superior esquerdo
            vertices[vertexIndex++] = x;
            vertices[vertexIndex++] = y + CELL_HEIGHT - CELL_SPACING;
            vertices[vertexIndex++] = 0.0f;

            // Cores
            float color[3];
            randomColor(color);

            // Seta as cores de todos os vertices
            for (int k = 0; k < 4; ++k)
            {
                colors[colorIndex++] = color[0];
                colors[colorIndex++] = color[1];
                colors[colorIndex++] = color[2];
             }
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(vertices));
}

void drawGrid()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);
    glDrawArrays(GL_QUADS, 0, GRID_SIZE * GRID_SIZE * 4 * 3);
    glBindVertexArray(0);
}


void findSimilarCells(int cell_i, int cell_j)
{
    if (cell_i > GRID_SIZE || cell_j > GRID_SIZE)
        return;

    int start_index = (cell_i + GRID_SIZE * cell_j) * 12;
    float base_r = colors[start_index] * 255;
    float base_g = colors[start_index + 1] * 255;
    float base_b = colors[start_index + 2] * 255;

    // Safe check
    if (base_r == 0.0f && base_g == 0.0f && base_b == 0.0f)
    {
        std::cout << "Movimento não permitido\n";
        return;
    }

    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            int start_index = (i * GRID_SIZE + j) * 12;
            float r = colors[start_index] * 255;
            float g = colors[start_index + 1] * 255;
            float b = colors[start_index + 2] * 255;

            float diff = sqrt(pow(r - base_r, 2) + pow(g - base_g, 2) + pow(b - base_b, 2));
            if (diff <= 0.20 * 255)
            {
                score++;
                for(int i = 0; i < 12; i++){
                    colors[start_index + i] = 0.0f;
                }

                std::cout << "\x1B[38;2;" << r << ";" << g << ";" << b << "m";
                std::cout << "▆ ";
                std::cout << "\x1B[0m"; // Reset color
            }
            else
            {
                std::cout << "  ";
            }

            if (j == GRID_SIZE - 1)
                std::cout << "\n";
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    drawGrid();

    moves--;
}


void keyboard(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
       createGrid();
       score = 0;
       moves = 5;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (moves > 0) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            float x = (xpos / WINDOW_WIDTH) * 2.0f - 1.0f;
            float y = 1.0f - (ypos / WINDOW_HEIGHT) * 2.0f;
            int cell_i = (int)((x + 1.0f - CELL_SPACING) / CELL_WIDTH);
            int cell_j = (int)((1.0f - y - CELL_SPACING / 2.0f) / CELL_HEIGHT);

            findSimilarCells(cell_i, cell_j);
            std::cout << "Pontuação: " << score << " Movimentos restantes: " << moves << "\n";
            if (moves == 0) {
                std::cout << "Jogo Finalizado, sua pontuaçao final foi " << score << ", aperte R para começar um novo jogo." << "\n";
            }
        }
        else
        {
            std::cout << "All moves used, game ended\n";
        }
    }
}

void initShaders()
{
   const char* vertex_shader =
    "#version 410\n"
    "in vec3 vp;"
    "in vec3 vc;"
    "out vec3 color;"
    "void main() {"
    "   gl_Position = vec4(vp, 1.0);"
    "   color = vc;"
    "}";

    const char* fragment_shader =
    "#version 410\n"
    "in vec3 color;"
    "out vec4 frg;"
    "void main () {"
    "   frg = vec4(color, 1.0);"
    "}";

    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader (vs);

    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, &fragment_shader, NULL);
    glCompileShader (fs);

    shader_programme = glCreateProgram();
    glAttachShader (shader_programme, fs);
    glAttachShader (shader_programme, vs);
    glBindAttribLocation(shader_programme, 0, "vp");
    glBindAttribLocation(shader_programme, 1, "vc");
    glLinkProgram(shader_programme);
}

int main()
{
    srand((unsigned int)time(NULL));

    GLFWwindow* window;

    // init GLFW
    glfwInit();

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Random Colors Grid", NULL, NULL);

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // init GLEW
    glewInit();

    // Crie a Grade
    createGrid();

    initShaders();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shader_programme);

        // Renderiza Grade
        drawGrid();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Inicializa glfw para receber entrada
        glfwPollEvents();

        // Entrada do mouse
        glfwSetMouseButtonCallback(window, mouseButtonCallback);

        // Entrada do teclado
        keyboard(window);
    }

    // Clean up
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwTerminate();
    return 0;
}
