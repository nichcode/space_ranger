
#include "pal2/pal_video.h"
#include "glm/glm.hpp"

int main(int argc, char** argv)
{
    palLog(nullptr, "Hello Space Ranger");
    glm::vec2 veca = {0, 2};
    glm::vec2 vecb = {4, 6};
    glm::vec2 vec3 = veca + vecb;

    return 0;
}