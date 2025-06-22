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
        utils::rand(-gl::window_aspect_ratio(), +gl::window_aspect_ratio()),
        utils::rand(-1.f, +1.f),
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
        return std::min(lifespan - age, 2.f) / 2.f * 0.03f;
    }

    float relative_age() const
    {
        return age / lifespan;
    }

    Particle()
    {
        float const initial_angle = utils::rand(0.f, 2.f * glm::pi<float>());

        velocity = {
            0.2f * std::cos(initial_angle),
            0.2f * std::sin(initial_angle),
        };
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

    std::vector<Particle> particles(50);

	glm::vec2 startVector1{-0.8f, 0.8f};
    glm::vec2 endVector1{0.8f, 0.8f};
	Vector2D vector_1(startVector1, glm::normalize(endVector1-startVector1), glm::length(endVector1-startVector1));

	glm::vec2 startVector2{-0.8f, -0.8f};
	glm::vec2 endVector2{0.8f, -0.8f};
	Vector2D vector_2(startVector2, glm::normalize(endVector2-startVector2), glm::length(endVector2-startVector2));

	glm::vec2 startVector3{-0.8f, 0.8f};
	glm::vec2 endVector3{-0.8f, -0.8f};
	Vector2D vector_3(startVector3, glm::normalize(endVector3-startVector3), glm::length(endVector3-startVector3));

	glm::vec2 startVector4{0.8f, 0.8f};
	glm::vec2 endVector4{0.8f, -0.8f};
	Vector2D vector_4(startVector4, glm::normalize(endVector4-startVector4), glm::length(endVector4-startVector4));

	std::vector<Vector2D> Lines;
	Lines.push_back(vector_1);
	Lines.push_back(vector_2);
	Lines.push_back(vector_3);
	Lines.push_back(vector_4);

    while (gl::window_is_open())
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& particle : particles)
        {
            // particle.age += gl::delta_time_in_seconds();

            auto forces = glm::vec2{0.f};

            // Gravity
            // forces += glm::vec2{0.f, -1.f} * particle.mass;

            // Air friction
            // forces += -particle.velocity * 1.f;

            // Follow mouse
            // forces += (gl::mouse_position() - particle.position);

            particle.velocity += forces / particle.mass * gl::delta_time_in_seconds();
            particle.position += particle.velocity * gl::delta_time_in_seconds();
			Vector2D particle_vector(particle.position, glm::normalize(particle.velocity), glm::length(particle.velocity));

			for (auto& line : Lines) {
				glm::vec2 intersection_position = defineLinesIntersection(line, particle_vector);
				if(intersection_position != glm::vec2(200.0f, 200.0f)) {
                    glm::vec2 line_normal = glm::vec2(-line.direction.y, line.direction.x);

					if (glm::dot(line_normal, particle.velocity) > 0.f) 
					{
						line_normal = -line_normal;
					}

					particle.velocity = glm::reflect(particle.velocity, line_normal);
				}
			}
        }

        std::erase_if(particles, [&](Particle const& particle) { return particle.age > particle.lifespan; });

        for (auto const& particle : particles)
            utils::draw_disk(particle.position, particle.radius(), glm::vec4{particle.color(), 1.f});

		


        float thickness = 0.005;
        glm::vec4 color{1.f, 1.f, 1.f, 1.f};
        
        // draw 2 lines with one define by mouse position : 
        // utils::draw_line(startVector1, endVector1,  thickness, color);
        // utils::draw_line(startVector2, endVector2, thickness, color);
		// utils::draw_line(startVector3, endVector3, thickness, color);
		// utils::draw_line(startVector4, endVector4, thickness, color);

		for (auto& line : Lines) {
			utils::draw_line(line.origin, line.origin+line.direction*line.magnitude,  thickness, color);
		}

		// glm::vec2 intersection_position = defineLinesIntersection(vector_1, vector_2);
        // utils::draw_disk(intersection_position, 0.025f, color);
    }
}