#include "view.h"

#include "cs123_lib/resourceloader.h"
#include "viewformat.h"
#include <QApplication>
#include <QKeyEvent>
#include <iostream>


View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_time(), m_timer(), m_captureMouse(false)
{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // Hide the cursor
    if (m_captureMouse) {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    }

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The update loop is implemented using a timer
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

View::~View()
{
    delete m_quad;
}

void View::initializeGL() {
    // All OpenGL initialization *MUST* be done during or after this
    // method. Before this method is called, there is no active OpenGL
    // context and all OpenGL calls have no effect.

    //initialize glew

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cerr << "Something is very wrong, glew initialization failed." << std::endl;
    }
    std::cout << "Using GLEW " <<  glewGetString( GLEW_VERSION ) << std::endl;

    // Start a timer that will try to get 60 frames per second (the actual
    // frame rate depends on the operating system and other running programs)
    m_time.start();
    m_timer.start(1000 / 60);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    initSim();
}

void View::initSim(){
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    m_rayProgram = ResourceLoader::createShaderProgram(
                ":/shaders/default.vert", ":/shaders/default.frag");
    glUseProgram(m_rayProgram);
    m_quad = new CS123::GL::FullScreenQuad();

    m_eye = glm::vec4(3.f, 5.f, 0.f, 1.f);
    m_view = glm::inverse(glm::lookAt(
        m_eye.xyz(),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f)
    ));
    GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    loc = glGetUniformLocation(m_rayProgram, "eye");
    glUniform4fv(loc, 1, glm::value_ptr(m_eye));

    //planet arrays being set up
    float radii[numPlanets] = {0, 1, 2, 3, 4};
    float colors[numPlanets*4] = {1, 1, 1, 1,
                                 1, 0, 0, 1,
                                 0, 0, 1, 1,
                                 1, 1, 0, 1,
                                 0, 1, 1, 1};

    //translate planets to the right by radius, and give a random rotation
    for (int i = 0; i < numPlanets; i++){
        if (i != 0){
            m_angularVels[i] = orbitalVelConstant*std::pow(radii[i], -3.f/2.f);
        } else {
            m_angularVels[i] = 0;
        }
        m_m[i] = glm::rotate(glm::mat4(1.f), 2*pi*std::rand(), glm::vec3(0.f, 1.f, 0.f));
        m_m[i] = glm::translate(m_m[i], glm::vec3(radii[i], 0.f, 0.f));
        m_invm[i] = glm::inverse(m_m[i]);
    }

    loc = glGetUniformLocation(m_rayProgram, "inv_m");
    glUniformMatrix4fv(loc, numPlanets, GL_FALSE, reinterpret_cast<float*>(&m_invm));
    loc = glGetUniformLocation(m_rayProgram, "m");
    glUniformMatrix4fv(loc, numPlanets, GL_FALSE, reinterpret_cast<float*>(&m_m));
    loc = glGetUniformLocation(m_rayProgram, "planet_colors");
    glUniform4fv(loc, numPlanets, reinterpret_cast<float*>(&colors));
}

void View::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_quad->draw();
    // TODO: Implement the demo rendering here
}

void View::resizeGL(int w, int h) {
    float ratio = static_cast<QGuiApplication *>(QCoreApplication::instance())->devicePixelRatio();
    w = static_cast<int>(w / ratio);
    h = static_cast<int>(h / ratio);
    glViewport(0, 0, w, h);
    GLint loc = glGetUniformLocation(m_rayProgram, "screenResolution");
    glUniform2f(loc, w, h);

    glm::mat4 p = glm::inverse(glm::perspective(
         glm::radians(45.0f), static_cast<float>(w)/static_cast<float>(h), 1.0f, 10.0f));
    loc = glGetUniformLocation(m_rayProgram, "inv_p");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(p));
    m_w = w;
    m_h = h;

    float k = 1;
    float vert = 2*k*glm::tan(camera_angle);
    float horiz = (vert*w)/h;
    glm::vec3 uvk = glm::vec3(horiz, vert, k);
    loc = glGetUniformLocation(m_rayProgram, "uvk");
    glUniform3fv(loc, 1, glm::value_ptr(uvk));
}

void View::mousePressEvent(QMouseEvent *event) {
}

void View::mouseMoveEvent(QMouseEvent *event) {
    // This starter code implements mouse capture, which gives the change in
    // mouse position since the last mouse movement. The mouse needs to be
    // recentered after every movement because it might otherwise run into
    // the edge of the screen, which would stop the user from moving further
    // in that direction. Note that it is important to check that deltaX and
    // deltaY are not zero before recentering the mouse, otherwise there will
    // be an infinite loop of mouse move events.
    if(m_captureMouse) {
        int deltaX = event->x() - width() / 2;
        int deltaY = event->y() - height() / 2;
        if (!deltaX && !deltaY) return;
        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
    }
}

void View::mouseReleaseEvent(QMouseEvent *event) {

}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) QApplication::quit();

    if (event->key() == Qt::Key_A){
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), step_angle_rotation, glm::vec3(0, 1, 0));
        m_view = m_view*rotate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }

    if (event->key() == Qt::Key_D){
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), -step_angle_rotation, glm::vec3(0, 1, 0));
        m_view = m_view*rotate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }

    if (event->key() == Qt::Key_W){
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), step_angle_rotation, glm::vec3(1, 0, 0));
        m_view = m_view*rotate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }

    if (event->key() == Qt::Key_S){
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), -step_angle_rotation, glm::vec3(1, 0, 0));
        m_view = m_view*rotate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }

    if (event->key() == Qt::Key_Up){
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -step_forward));
        m_view = m_view*translate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }

    if (event->key() == Qt::Key_Down){
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, step_forward));
        m_view = m_view*translate;

        GLint loc = glGetUniformLocation(m_rayProgram, "inv_v");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_view));
    }
}

void View::keyReleaseEvent(QKeyEvent *event) {

}
void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    float seconds = m_time.restart() * 0.001f;

    //rotate planets depending on their angular velocity
    for (int i = 0; i < numPlanets; i++){
        m_invm[i] = glm::rotate(m_invm[i], seconds*m_angularVels[i], glm::vec3(0, 1, 0));
        m_m[i] = glm::inverse(m_invm[i]);
    }

    GLint loc = glGetUniformLocation(m_rayProgram, "inv_m");
    glUniformMatrix4fv(loc, numPlanets, GL_FALSE, reinterpret_cast<float*>(&m_invm));
    loc = glGetUniformLocation(m_rayProgram, "m");
    glUniformMatrix4fv(loc, numPlanets, GL_FALSE, reinterpret_cast<float*>(&m_m));

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}
