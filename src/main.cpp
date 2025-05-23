#include "opengl-framework/opengl-framework.hpp"
#include "utils.hpp"

struct Particle {
    glm::vec2 position = glm::vec2(utils::rand(-gl::window_aspect_ratio(), +gl::window_aspect_ratio()), utils::rand(-1.f, 1.f));
    float radius = .01f; 
    glm::vec4 color = glm::vec4(.2f, .2f, 1.f, 1.f);
    glm::vec2 speedVector = determineVectorFromPolarCoordinates();

    glm::vec2 determineVectorFromPolarCoordinates() {
        float angle = utils::rand(0.f, 360.f);
        float vectorLength = utils::rand(0.01f, 0.05f);

        return glm::vec2(glm::cos(angle)*vectorLength, glm::sin(angle)*vectorLength);
    };

    void moveParticle(float deltaTime) {
        position.x += speedVector.x * deltaTime;
        position.y += speedVector.y * deltaTime;
    };
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

        // TODO update particles
        // TODO render particles
        for(Particle& particle : particleArray){
            particle.moveParticle(gl::delta_time_in_seconds());
            utils::draw_disk(particle.position, particle.radius, particle.color);
        }
    }
}