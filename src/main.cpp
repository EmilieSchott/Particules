#include "opengl-framework/opengl-framework.hpp"
#include "utils.hpp"

struct Particle {
    glm::vec2 position = glm::vec2(utils::rand(-gl::window_aspect_ratio(), +gl::window_aspect_ratio()), utils::rand(-1, 1));
    // int x = utils::rand(-gl::window_aspect_ratio(), +gl::window_aspect_ratio());
    // int y = utils::rand(-1, 1);
    float radius = .01f; 
    glm::vec4 color = glm::vec4(.2f, .2f, 1.f, 1.f);
};

int main()
{
    gl::init("Particules!");
    gl::maximize_window();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // TODO: create an array of particles
    std::vector<Particle> particleArray;
    for(int i = 0; i < 100; ++i){
        Particle particle;
        particleArray.push_back(particle);
    }

    while (gl::window_is_open())
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        for(Particle particle : particleArray){
            utils::draw_disk(particle.position, particle.radius, particle.color);
        }

        // TODO update particles
        // TODO render particles
    }
}