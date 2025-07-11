#include "glm/ext/scalar_constants.hpp"
#include "opengl-framework/opengl-framework.hpp"
#include "utils.hpp"

float easeInOut(float x, float power)
{
    if (x < 0.5)
    {
        return 0.5 * pow(2 * x, power);
    }
    else
    {
        return 1 - 0.5 * pow(2 * (1 - x), power);
    }
}

struct Particle {


    glm::vec2 position{
    };

    glm::vec2 velocity;

    float mass{utils::rand(1.f, 2.f)};

    float age{0.f};
    float lifespan{utils::rand(3.f, 5.f)};

    glm::vec3 start_color{
        utils::rand(0.f, 1.f),
        utils::rand(0.f, 1.f),
        utils::rand(0.f, 1.f),
    };
    glm::vec3 end_color{
        utils::rand(0.f, 1.f),
        utils::rand(0.f, 1.f),
        utils::rand(0.f, 1.f),
    };

    glm::vec3 color() const
    {
        return glm::mix(start_color, end_color, easeInOut(relative_age(), 4.f));
    }

    float radius() const
    {
        return 0.01f;
    }

    float relative_age() const
    {
        return age / lifespan;
    }

    Particle(float square_with)
    {
        float x = square_with + 10 ;
        float y = square_with + 10;

        while(std::hypot(x, y) > square_with/2) {
            x = utils::rand(-square_with/2, +square_with/2);
            y = utils::rand(-square_with/2, +square_with/2);
        }

        position = glm::vec2{x, y};
    }
};

struct Vector2D {
	Vector2D(glm::vec2 vector_origin, glm::vec2 vector_direction, float vector_magnitude)
	{
		origin = vector_origin;
		direction = vector_direction;
		magnitude = vector_magnitude;
	};

	glm::vec2 origin;
	glm::vec2 direction;
	float magnitude;
};

 glm::vec2 defineLinesIntersection(Vector2D vector_1, Vector2D vector_2) {

    glm::mat2 matrix = glm::mat2(vector_1.direction, -vector_2.direction);
	glm::mat2 inversed_matrix = glm::inverse(matrix);

    glm::vec2 origin_vector = glm::vec2(vector_2.origin - vector_1.origin);

    glm::vec2 intersection_vector =  inversed_matrix * origin_vector;

	if(intersection_vector.x < 0.0f || intersection_vector.x > 1.0f || intersection_vector.y < 0.0f || intersection_vector.y > vector_2.magnitude) {
		return glm::vec2(200.0f, 200.0f);
	}

	return vector_1.origin + intersection_vector.x * vector_1.direction;

}

int main()
{
    gl::init("Particules!");
    gl::maximize_window();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    std::vector<Particle> particles{};
    for (int i = 0; i < 1500; i++)
    {
        Particle particle = Particle(0.5f);
        particles.push_back(particle);
    }

    while (gl::window_is_open())
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto const& particle : particles)
            utils::draw_disk(particle.position, particle.radius(), glm::vec4{particle.color(), 1.f});

        float thickness = 0.005;
        glm::vec4 color{1.f, 1.f, 1.f, 1.f};
    }
}