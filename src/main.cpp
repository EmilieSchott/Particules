#include <cstddef>
#include <iostream>
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/geometric.hpp"
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

struct Wall {
    glm::vec2 start{};
    glm::vec2 end{};
};

struct Segment {
    glm::vec2 start{};
    glm::vec2 end{};
};

bool nearly_equal(float a, float b, float epsilon = 1e-6f)
{
    return std::fabs(a - b) < epsilon;
}

bool is_invertible(const glm::mat2& mat, float epsilon = 1e-6f)
{
    return !nearly_equal(glm::determinant(mat), 0.0f, epsilon);
}

struct Circle {
    glm::vec2 center;
    float     radius;
};

std::optional<glm::vec2> intersection(Segment const& segment, Circle const& circle)
{
    glm::vec2 const o      = segment.start;
    glm::vec2 const d      = segment.end - segment.start;
    glm::vec2 const center = circle.center;
    float const     r      = circle.radius;

    float const a = glm::dot(d, d);
    float const b = 2 * glm::dot(o - center, d);
    float const c = glm::dot(o - center, o - center) - r * r;

    float const delta = b * b - 4 * a * c;
    if (delta <= 0)
        return std::nullopt;

    float const t1 = (-b - std::sqrt(delta)) / 2.f / a;
    float const t2 = (-b + std::sqrt(delta)) / 2.f / a;

    if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)
        return o + std::min(t1, t2) * d;

    if (0 <= t1 && t1 <= 1)
        return o + t1 * d;

    if (0 <= t2 && t2 <= 1)
        return o + t2 * d;

    return std::nullopt;
}

std::optional<glm::vec2> intersection(Segment const& s1, Segment const& s2)
{
    glm::vec2 const o1 = s1.start;
    glm::vec2 const o2 = s2.start;
    glm::vec2 const d1 = s1.end - s1.start;
    glm::vec2 const d2 = s2.end - s2.start;

    glm::mat2x2 const mat = glm::mat2x2{d1, -d2};
    // if (!is_invertible(mat))
    //     return std::nullopt;

    glm::vec2 const solutions = glm::inverse(mat) * (o2 - o1);
    if (0 <= solutions.x && solutions.x <= 1 && 0 <= solutions.y && solutions.y <= 1)
        return o1 + solutions.x * d1;

    return std::nullopt;
}

struct Grid {
    Grid(size_t width, size_t height)
    {
        indices.resize(width);
        for (auto& column : indices)
            column.resize(height);
    }

    auto operator()(size_t x, size_t y) -> std::optional<size_t>&
    {
        return indices[x][y];
    }

    auto width() const -> size_t
    {
        return indices.size();
    }

    auto height() const -> size_t
    {
        return indices[0].size();
    }

    std::vector<std::vector<std::optional<size_t>>> indices;
};

auto poisson_disk_sampling(glm::vec2 region_min, glm::vec2 region_max, float spacing, int max_attemps) -> std::vector<glm::vec2>
{
    auto        all_points    = std::vector<glm::vec2>{};
    auto        active_points = std::vector<glm::vec2>{(region_min + region_max) * 0.5f};
    float const cell_size     = spacing / std::sqrt(2.f);
    auto const  region_size   = region_max - region_min;
    auto const  bob           = region_size / cell_size;
    auto        grid          = Grid(std::ceil(bob.x), std::ceil(bob.y));

    while (!active_points.empty())
    {
        auto const  start_point_index = utils::rand(0.f, active_points.size() - 1);
        auto const& start_point       = active_points[start_point_index];

        bool is_valid{false};
        for (int attempt = 0; attempt < max_attemps; ++attempt)
        {
            auto const angle     = utils::rand(0.f, 2.f * glm::pi<float>());
            auto const radius    = utils::rand(spacing, 2.f * spacing);
            auto const new_point = start_point + radius * glm::vec2{cos(angle), sin(angle)};
            bool       blah{true};
            if (new_point.x < region_min.x || new_point.x > region_max.x
                || new_point.y < region_min.y || new_point.y > region_max.y)
            {
                blah = false;
            }

            auto const new_point_indices = glm::floor((new_point - region_min) / cell_size);

            for (int x = new_point_indices.x - 2; x <= new_point_indices.x + 2; ++x)
            {
                for (int y = new_point_indices.y - 2; y <= new_point_indices.y + 2; ++y)
                {
                    if (x < 0 || x >= grid.width() || y < 0 || y >= grid.height())
                        continue;
                    auto const idx = grid(x, y);
                    if (!idx.has_value())
                        continue;
                    auto const point = all_points[*idx];
                    if (glm::distance(point, new_point) < spacing)
                    {
                        blah = false;
                    }
                }
            }
            if (blah)
            {
                active_points.push_back(new_point);
                all_points.push_back(new_point);
                grid(new_point_indices.x, new_point_indices.y) = all_points.size() - 1;
                is_valid                                       = true;
                break;
            }
        }
        if (!is_valid)
            active_points.erase(active_points.begin() + start_point_index);
    }

    return all_points;
}

struct Particle {
    glm::vec2 position{};

    glm::vec2 velocity{0.f};

    float age{0.f};
    float lifespan{utils::rand(30.f, 50.f)};

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
        // return glm::mix(start_color, end_color, easeInOut(relative_age(), 4.f));
        // return glm::vec3{1.f}; 
        return glm::vec3{1.f, 0.f, 0.f}; // rouge
    }

    float radius() const
    {
        return 0.03f / 2.f; // std::min(lifespan - age, 2.f) / 2.f * 0.03f;
    }

    float relative_age() const
    {
        return age / lifespan;
    }

    Particle(glm::vec2 bezierPosition, glm::vec2 bezierNormal)
    {
        // Rectangle
        // position.x = utils::rand(0.3, 0.8);
        // position.y = utils::rand(-0.2, 0.5);

        // Parallélogramme
        // auto const x_axis = glm::vec2{0.5f, 0.f};
        // auto const y_axis = glm::vec2{0.7f, 0.7f};
        // auto const offset = glm::vec2{0.5f, -0.2f};
        // position    = utils::rand(0.f, 1.f) * x_axis
        //            + utils::rand(0.f, 1.f) * y_axis
        //            + offset;

        // Disque
        // auto const angle  = utils::rand(0.f, 2.f * glm::pi<float>());
        // auto const area   = utils::rand(0.f, glm::pi<float>() * 0.8f * 0.8f);
        // auto const radius = std::sqrt(area / glm::pi<float>());
        // position          = radius * glm::vec2{cos(angle), sin(angle)};

        // Disque rejection sampling
        // while (true)
        // {
        //     float const R = 0.8f;
        //     position.x    = utils::rand(-R, R);
        //     position.y    = utils::rand(-R, R);
        //     if (glm::length(position) < R)
        //         break;
        // }

        // Bézier :
        position = bezierPosition;
        velocity = 0.3f * bezierNormal;
    }
};

/* Ceci est une fonction qui retourne un vec2 et prend en paramètre un float */
void draw_parametric(std::function<glm::vec2(float)> const& parametric)
{
    int steps = 100;
    glm::vec2 start = parametric(0.f);


    for (int i = 1; i <= steps; ++i)
    { 
        float t = static_cast<float>(i) / steps;
        glm::vec2 end = parametric(t);
        utils::draw_line(start, end, 0.01f, glm::vec4{1.f, 1.f, 1.f, 1.f});
        start = end;
    }
}

glm::vec2 bezier3(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t)
{
    float u = 1.f - t;
    return u*u*u * p0
        + 3.f * u*u * t * p1
        + 3.f * u * t*t * p2
        + t*t*t * p3;
}

glm::vec2 bezier3_derivative(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t)
{
    float u = 1.f - t;
    return 
        3.f * u*u * (p1 - p0) +
        6.f * u * t * (p2 - p1) +
        3.f * t*t * (p3 - p2);
}

float find_closest_t_on_bezier(glm::vec2 mouse, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    float best_t = 0.f;
    float min_distance = std::numeric_limits<float>::max();

    // On teste plusieurs t initiaux pour éviter les minima locaux
    for (int i = 0; i <= 10; ++i)
    {
        float t = static_cast<float>(i) / 10.f;
        float learning_rate = 0.01f;

        // Descente de gradient
        for (int i = 0; i < 20; ++i)
        {
            glm::vec2 Bezier = bezier3(p0, p1, p2, p3, t);
            glm::vec2 derivativeBezier = bezier3_derivative(p0, p1, p2, p3, t);

            glm::vec2 distance = Bezier - mouse;

            float gradient = 2.f * glm::dot(distance, derivativeBezier);

            t -= learning_rate * gradient;
            t = glm::clamp(t, 0.f, 1.f); // rester entre 0 et 1
        }

        glm::vec2 final_point = bezier3(p0, p1, p2, p3, t);
        float distance = glm::distance(final_point, mouse);

        if (distance < min_distance)
        {
            min_distance = distance;
            best_t = t;
        }
    }

    return best_t;
}

int main()
{
    gl::init("Particules!");
    gl::maximize_window();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // auto const points = poisson_disk_sampling({-1.f, -1.f}, {1.f, 1.f}, 0.1f, 20);

    // std::vector<Particle> particles(points.size());
    // for (size_t i = 0; i < points.size(); ++i)
    //     particles[i].position = points[i];
    
    glm::vec2 p0 = {-.3f, -.3f};
    glm::vec2 p1 = {-0.2f, 0.5f};
    glm::vec2 p2 = {0.7f, -0.8f};
    glm::vec2 p3 = {.8f, .5f};

    std::vector<Particle> particles{};
    int particleAmount = 50;
    for (int i = 0; i < particleAmount; i++)
    {
        float t = (float)1/(float)particleAmount * (float)i;
        glm::vec2 position = bezier3(p0, p1, p2, p3, t);

        glm::vec2 tangent = bezier3_derivative(p0, p1, p2, p3, t);
        glm::vec2 normal = glm::normalize(glm::vec2(-tangent.y, tangent.x));

        Particle particle = Particle(position, normal);
        particles.push_back(particle);
    }

    while (gl::window_is_open())
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& particle : particles)
        {
        //     particle.age += gl::delta_time_in_seconds();

        //     auto forces = glm::vec2{0.f};

            // Gravity
            // forces += glm::vec2{0.f, -1.f} * particle.mass;

            // Air friction
            // forces += -particle.velocity * 1.f;

            // Follow mouse
            // forces += (gl::mouse_position() - particle.position);

        //     particle.velocity += forces * gl::delta_time_in_seconds();
            particle.position += particle.velocity * gl::delta_time_in_seconds();
        }


        draw_parametric([](float t) {
            return bezier3({-.3f, -.3f}, {-0.2f, 0.5f}, {0.7f, -0.8f}, {.8f, .5f}, t);
        });

        
        // draw particles :
        // for (auto const& particle : particles) {
        //     utils::draw_disk(particle.position, particle.radius(), glm::vec4{particle.color(), 1.f});
        // }
        // std::erase_if(particles, [&](Particle const& particle) { return particle.age > particle.lifespan; });

        glm::vec2 mouse = gl::mouse_position();

        float t_closest = find_closest_t_on_bezier(mouse, p0, p1, p2, p3);
        glm::vec2 closest_point = bezier3(p0, p1, p2, p3, t_closest);

        // dessine le point le plus proche en bleu :
        utils::draw_disk(closest_point, 0.02f, glm::vec4{0.f, 0.f, 1.f, 1.f});
    }
}