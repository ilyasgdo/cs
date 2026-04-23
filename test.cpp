#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

void Render()
{
 glClearColor(1.f, 1.f, 0.f, 1.f);
 glClear(GL_COLOR_BUFFER_BIT);
 glBegin(GL_TRIANGLES);
 glVertex2f(0.0f, 0.5f);
 glVertex2f(0.5f, -0.5f);
 glVertex2f(-0.5f, -0.5f);
 glEnd();
}
// la ‘callback’ executee par glutDisplayFunc()
void Display()
{
 Render();
 glutSwapBuffers();
}
int main(int argc, char** argv)
{
 glutInit(&argc, argv);
 glutInitDisplayMode(GLUT_DOUBLE);
 glutInitWindowSize(960,540);
 glutCreateWindow("Triangle");
 glutDisplayFunc(Display);
 glutMainLoop();
 return 0;
}