#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct Vector3f { // Структура трехмерной точки
    float x;
    float y;
    float z;

    Vector3f() {}

    Vector3f(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }
};

GLuint VBO; // Указатель на буфер вершин

static void RenderSceneCB() { // Функция, отвечающая за сцену
    glClear(GL_COLOR_BUFFER_BIT); // Очистка кадра

    glEnableVertexAttribArray(0); // Включение атрибутов вершин
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Указание, что VBO - это массив вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0); // Указание как воспринимать данные из буфера

    glDrawArrays(GL_TRIANGLES, 0, 6); // Отрисовка элементов буфера вершин как треугольников

    glDisableVertexAttribArray(0); // Выключение атрибутов вершин

    glutSwapBuffers(); // Переключение на буферный кадр
}


static void InitializeGlutCallbacks() {
    glutDisplayFunc(RenderSceneCB); // Функция отрисовки сцены
}

static void CreateVertexBuffer() {
    Vector3f Vertices[6]; // Инициализация и заполнение буфера на 6 вершин
    Vertices[0] = Vector3f(-0.5f, -0.25f, 0.0f);
    Vertices[1] = Vector3f(0.5f, -0.25f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 0.55f, 0.0f);

    Vertices[3] = Vector3f(0.5f, 0.25f, 0.0f);
    Vertices[4] = Vector3f(-0.5f, 0.25f, 0.0f);
    Vertices[5] = Vector3f(0.0f, -0.55f, 0.0f);

    glGenBuffers(6, &VBO); // Иницализация размера буфера объектов
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Указание, что VBO - это массив вершин
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // Наполнение VBO элементами из Vertices
} 


int main(int argc, char** argv) {
    glutInit(&argc, argv); // Инициализация GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(480, 480); // Инициализация параметров окна
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Star of David");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit(); // Инициализаия GLEW
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.25f, 0.5f, 1.0f, 0.0f); // Цвет окна

    CreateVertexBuffer(); // Создание буфера вершина 

    glutMainLoop(); // Цикл отрисовки кадров

    return 0;
}
