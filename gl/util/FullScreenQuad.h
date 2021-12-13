#ifndef FULLSCREENQUAD_H
#define FULLSCREENQUAD_H

#include "gl/datatype/VAO.h"
#include <memory>

namespace CS123 { namespace GL {

class VAO;

class FullScreenQuad {
public:
    FullScreenQuad();

    void draw();

private:
    std::unique_ptr<VAO> m_vao;
};

}}

#endif // FULLSCREENQUAD_H
