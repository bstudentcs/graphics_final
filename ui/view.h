#ifndef VIEW_H
#define VIEW_H

#include "glm/glm.hpp"            // glm::vec*, mat*, and basic glm functions
#include "glm/gtx/transform.hpp"  // glm::translate, scale, rotate
#include "glm/gtc/type_ptr.hpp"   // glm::value_ptr


#include "gl/util/FullScreenQuad.h"
#include "GL/glew.h"
#include <memory>
#include <cstdlib>
#include <qgl.h>
#include <QTime>
#include <QTimer>

const int numPlanets = 5;
const float pi = 3.14159265358;
const float orbitalVelConstant = 0.2f; //changes angular velocity of planets

class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

private:
    QTime m_time;
    QTimer m_timer;
    bool m_captureMouse;
    float m_w, m_h;
    GLuint m_rayProgram;
    CS123::GL::FullScreenQuad* m_quad;
    glm::mat4 m_m[numPlanets];
    float m_angularVels[numPlanets];
    glm::vec4 m_eye;

    void initializeGL();
    void paintGL();
    void initSim();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void tick();
};

#endif // VIEW_H
