#define GL_SILENCE_DEPRECATION
#include<OpenGL/gl.h>
#include<GLUT/glut.h>
#include<OpenGL/glu.h>


int windowWidth = 960;
int windowHeight = 1000;

void drawScene()
{
    glBegin(GL_QUADS);
        glColor3f(0.3f, 0.7f, 0.3f);
        glVertex3f(-5.0f, 0.0f, -5.0f);
        glVertex3f(5.0f, 0.0f, -5.0f);
        glVertex3f(5.0f, 0.0f, 5.0f);
        glVertex3f(-5.0f, 0.0f, 5.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 2.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 2.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 2.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);

        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 2.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
    glEnd();

    glBegin(GL_QUADS);
        glColor3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);
    glEnd();
}

void Render()
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / windowHeight, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 8.0, 2.0,
              0.0, 0.0, 0.0,
              0.0, 0.0, 1.0);

    drawScene();
}

void Display()
{
    Render();
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("EXO 5.1");
    glutDisplayFunc(Display);
    glutMainLoop();
    return 0;
}