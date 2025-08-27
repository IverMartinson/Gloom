#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include "include/sdl_draw.cpp"
#include "include/SDL_func.cpp"
#include "include/geometry.cpp"
#include "include/time.cpp"

using namespace std;

double PI = 3.14159265358979323846;

struct Player{
    vector2 v;
    vector2 p;
    double angle;
    double angle_rad;
    double fov;
    double resolution;
    double view_distance = 4000;

    Player(vector2 position, double fov, double resolution) : p(position), fov(fov), resolution(resolution) {}
};

struct Line{
    vector2 point1;
    vector2 point2;
    double angle;
    double slope;
    double min_x;
    double max_x;
    bool is_mirror;
    Color color;

    Line(vector2 point1 = vector2(0, 0), vector2 point2 = vector2(0, 0), bool is_mirror = false) : point1(point1), point2(point2), is_mirror(is_mirror), slope((point1.y - point2.y) / (point1.x - point2.x)), angle(atan2(-(point1.y - point2.y), -(point1.x - point2.x)) * 180 / PI) {}
};

struct Ray{
    vector2 position;
    double original_angle;
    double angle;
    double angle_rad;
    int max_reflections;
    bool intersecting = false;
    vector2 intersection;
    vector<vector2> intersections;
    vector<double> reflection_angles;
    vector2 reflected_intersection;
    double reflection_angle;
    bool is_reflecting;
    double distance;
    double total_distance;
    Line intersected_line;

    Ray(vector2 position, double angle, int max_reflections) : position(position), original_angle(angle), angle(angle), max_reflections(max_reflections) {}
};

vector<Line> lines;
vector<Ray> rays;

int half_width = 450;
int half_height = 450;

void loop(double& value){
    if (value >= 360) value -= 360;
    if (value < 0) value += 360;
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = surface->w;
    destRect.h = surface->h;

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

SDL_Renderer* renderer;

void raycast(Ray& ray){
    ray.angle_rad = ray.angle * PI / 180;

    ray.reflection_angles.clear();
    ray.intersections.clear();
    ray.intersecting = false;
    vector<Line> intersected_lines;

    loop(ray.angle);

    for (Line line : lines){
        //if (line.)

        double ray_slope = tan(ray.angle_rad);

        double ray_b = ray_slope * -ray.position.x + ray.position.y;
        double line_b = line.slope * -line.point1.x + line.point1.y;

        double x = (ray_b - line_b) / (line.slope - ray_slope);
        double y = ray_slope * x + ray_b;

        if ((ray.angle < 180 && x >= line.min_x && x <= line.max_x && y > ray.position.y) || (ray.angle >= 180 && x >= line.min_x && x <= line.max_x && y < ray.position.y)){
            ray.intersecting = true;

            ray.intersections.push_back(vector2(x, y));

            intersected_lines.push_back(line);

            ray.reflection_angles.push_back(line.angle * 2 - ray.angle);
        }
    }

    if (ray.intersecting){
        int lowest_index = 0;
        double lowest_distance = 1000000;

        for (int i = 0; i < ray.intersections.size(); ++i){
            double distance = distance_2d_v(ray.position, ray.intersections[i]);

            if (distance < lowest_distance){
                lowest_distance = distance;
                lowest_index = i;
            }
        }

        ray.intersection = ray.intersections[lowest_index];
        ray.is_reflecting = intersected_lines[lowest_index].is_mirror;
        ray.reflection_angle = ray.reflection_angles[lowest_index];
        ray.total_distance = lowest_distance;
        ray.intersected_line = intersected_lines[lowest_index];
    }
}

void open_map(float scale=1){
    ifstream file("map.crm");
    
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            vector<int> points;
            string token;
            string wall_type;
            while (getline(ss, token, ';')) {
                stringstream ss2(token);
                string coord;
                while (getline(ss2, coord, ':')) {
                    points.push_back(stoi(coord));
                    stringstream ss2(token);
                }
            }

            bool is_mirror = (line.find("Mirror") != string::npos);

            Line new_line(vector2(points[0] * scale, points[1] * scale), vector2(points[2] * scale, points[3] * scale), is_mirror);
            if (is_mirror)new_line.color = Color(0, 255, 255);
            else new_line.color = Color(116, 116, 116);

            lines.push_back(new_line);
        }
        file.close();
    }
}

bool debug = false;

int main(int argc, char* argv[]) {
    int lastTick = now();
    double dt = 0;

    Player player(vector2(0, 0), 90, 900);

    double increment = player.fov / player.resolution;
    for (double i = player.fov - increment / 2; i > 0; i -= increment){
        rays.push_back(Ray(player.p, i - player.fov / 2, 100));
    }

    init();

    TTF_Init();

    TTF_Font* font = TTF_OpenFont("GaMaamli-Regular.ttf", 24);

    open_map();

    // make_polygon(20, 450);
    // make_spiral(10, 200);

    for (Line& line : lines){
        loop(line.angle);
        if (line.point1.x <= line.point2.x) {
            line.min_x = line.point1.x;
            line.max_x = line.point2.x;
        } else {
            line.min_x = line.point2.x;
            line.max_x = line.point1.x;
        }

        if (line.angle == 90){
            Color c = line.color;
            line = Line(line.point1 - 0.00001, line.point2, line.is_mirror);
            line.color = c;
        } else if (line.angle == 270){
            Color c = line.color;
            line = Line(line.point2 + 0.00001, line.point1, line.is_mirror);
            line.color = c;
        }

        if (line.point1.x <= line.point2.x) {
            line.min_x = line.point1.x;
            line.max_x = line.point2.x;
        } else {
            line.min_x = line.point2.x;
            line.max_x = line.point1.x;
        }
    }

    SDL_Window* window_gloom = create_window("gloom", half_width * 2, half_height * 2);
    SDL_Renderer* renderer_gloom = create_renderer(window_gloom);

    SDL_Window* window = create_window("raycasting engine", half_width * 2, half_height * 2);
    SDL_Renderer* renderer = create_renderer(window);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(renderer_gloom, SDL_BLENDMODE_BLEND);

    SDL_SetWindowPosition(window, 0 + 1920, 90);
    SDL_SetWindowPosition(window_gloom, 1020 + 1920, 90);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    bool running = true;
    
    double move_speed = 500;

    SDL_Event event;
    while (running){
        dt = (now() - lastTick) / 1000000000.0;
        lastTick = now();

        SDL_WarpMouseInWindow(window, half_width, half_height);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEMOTION) {
                int mouse_delta_x = event.motion.xrel;
                player.angle -= mouse_delta_x * 0.06;
                loop(player.angle);
                player.angle_rad = player.angle * PI / 180;
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

        if (currentKeyStates[SDL_SCANCODE_ESCAPE]) running = false;

        player.p = player.p + player.v * dt * dt;

        if (currentKeyStates[SDL_SCANCODE_W]) {
            player.v = player.v + vector2(cos(player.angle_rad), sin(player.angle_rad)) * move_speed;
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            player.v = player.v - vector2(cos(player.angle_rad), sin(player.angle_rad)) * move_speed;
        }
        if (currentKeyStates[SDL_SCANCODE_A]) {
            player.v = player.v - vector2(sin(player.angle_rad), -cos(player.angle_rad)) * move_speed;
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            player.v = player.v + vector2(sin(player.angle_rad), -cos(player.angle_rad)) * move_speed;
        }

        player.v = player.v * 0.9;

        clear_window(renderer, Color(0, 0, 0));
        clear_window(renderer_gloom, Color(0, 0, 0));

        Color ray_color(245, 169, 184);
        Color wall_color(116, 116, 116);
        Color mirror_color(0, 255, 255);

        char angle_text[32];
        snprintf(angle_text, sizeof(angle_text), "%.0f", 1 / dt);
        render_text(renderer, font, angle_text, 0, 0, {255, 255, 255});

        for (Line line : lines){
            draw_line(renderer, vector<int>{half_width, half_height}, line.point1.x, line.point1.y, line.point2.x, line.point2.y, line.color);
            if (!debug) continue;
            char angle_text[32];
            snprintf(angle_text, sizeof(angle_text), "%.0f", line.angle);
            render_text(renderer, font, angle_text, (line.point1.x + line.point2.x) / 2 + half_width, -(line.point1.y + line.point2.y) / 2 + half_height, {255, 255, 255});
        }

        for (Ray& ray : rays){
            ray.position = player.p;
            ray.angle = ray.original_angle + player.angle;

            ray.total_distance = 0;

            if(debug){char angle_text[32];
            snprintf(angle_text, sizeof(angle_text), "%.4f", ray.angle);
            render_text(renderer, font, angle_text, ray.position.x + half_width, -ray.position.y + half_height, {255, 255, 255});}

            raycast(ray);

            ray.distance = ray.total_distance;

            if (!ray.intersecting) ray.total_distance = 1000000000;

            if (ray.max_reflections > 0 && ray.intersecting && ray.is_reflecting){
                vector2 off_the_line_mark_exclamation_point(cos(ray.reflection_angle * PI / 180) * 0.001, sin(ray.reflection_angle * PI / 180) * 0.001);

                Ray reflection_ray = Ray(ray.intersection + off_the_line_mark_exclamation_point, ray.reflection_angle, 0);

                raycast(reflection_ray);

                ray.total_distance += reflection_ray.total_distance;

                if(debug){for (vector2 intersection : reflection_ray.intersections){draw_rect_filled(renderer, vector<int>{half_width, half_height}, intersection.x, intersection.y, 7, 7, Color(255, 0, 0));}char angle_text[32];
                snprintf(angle_text, sizeof(angle_text), "%.4f", ray.reflection_angle);
                render_text(renderer, font, angle_text, ray.position.x + half_width - 120, -ray.position.y + half_height, {255, 255, 255});
                snprintf(angle_text, sizeof(angle_text), "%.4f", reflection_ray.angle);
                render_text(renderer, font, angle_text, reflection_ray.position.x + half_width, -reflection_ray.position.y + half_height, {255, 255, 255});}

                if (reflection_ray.intersecting && reflection_ray.is_reflecting)for(int i = 1; i < ray.max_reflections; ++i){
                    vector2 far_position_reflection = vector2(cos(reflection_ray.angle_rad) * 700, sin(reflection_ray.angle_rad) * 700);
if (reflection_ray.is_reflecting)draw_line(renderer, vector<int>{half_width, half_height}, reflection_ray.position.x, reflection_ray.position.y, reflection_ray.intersection.x, reflection_ray.intersection.y, ray_color);

                    if (!reflection_ray.intersecting || !reflection_ray.is_reflecting)                    draw_line(renderer, vector<int>{half_width, half_height}, reflection_ray.position.x, reflection_ray.position.y, reflection_ray.intersecting ? reflection_ray.intersection.x : far_position_reflection.x, reflection_ray.intersecting ? reflection_ray.intersection.y : far_position_reflection.y, Color(255, 0, 0));
   
                    vector2 off_the_line_mark_exclamation_point(cos(reflection_ray.reflection_angle * PI / 180) * 0.001, sin(reflection_ray.reflection_angle * PI / 180) * 0.001);

                    reflection_ray = Ray(reflection_ray.intersection + off_the_line_mark_exclamation_point, reflection_ray.reflection_angle, 0);

                    raycast(reflection_ray);
                    
                    if (!reflection_ray.intersecting || !reflection_ray.is_reflecting) break;

                    ray.total_distance += reflection_ray.total_distance;

                    if(debug){char angle_text[32];
                    snprintf(angle_text, sizeof(angle_text), "%.4f", reflection_ray.angle);
                    render_text(renderer, font, angle_text, reflection_ray.position.x + half_width, -reflection_ray.position.y + half_height, {255, 255, 255});}

                    if (debug)for (vector2 intersection : reflection_ray.intersections){draw_rect_filled(renderer, vector<int>{half_width, half_height}, intersection.x, intersection.y, 7, 7, Color(255, 0, 0));}
                }
                

                if (reflection_ray.is_reflecting)draw_line(renderer, vector<int>{half_width, half_height}, reflection_ray.position.x, reflection_ray.position.y, reflection_ray.intersection.x, reflection_ray.intersection.y, ray_color);
                    if (!reflection_ray.intersecting || !reflection_ray.is_reflecting)                    draw_line(renderer, vector<int>{half_width, half_height}, reflection_ray.position.x, reflection_ray.position.y, reflection_ray.intersection.x, reflection_ray.intersection.y, Color(255, 0, 0));


                ray.intersected_line = reflection_ray.intersected_line;
                ray.reflected_intersection = reflection_ray.intersection;
            }

            if(ray.intersecting)draw_line(renderer, vector<int>{half_width, half_height}, ray.position.x, ray.position.y, ray.intersection.x, ray.intersection.y, ray_color);

            if(debug)for (vector2 intersection : ray.intersections){draw_rect_filled(renderer, vector<int>{half_width, half_height}, intersection.x, intersection.y, 7, 7, Color(255, 0, 0));}
        }

        double increment = 900 / player.resolution;
        for (double i = -450; i < 450; i += increment){
            draw_line(renderer_gloom, vector<int>{half_width, half_height}, i, 0, i, 450, Color(81, 83, 90));
            draw_line(renderer_gloom, vector<int>{half_width, half_height}, i, 0, i, -450, Color(100, 80, 70));
            Ray ray = rays[i + 450];
            if (ray.total_distance > player.view_distance) continue;
            double height = player.view_distance / (ray.total_distance * cos(ray.angle_rad - player.angle_rad));
            int r = ray.intersected_line.color.r / player.view_distance * abs(ray.total_distance - player.view_distance);
            int g = ray.intersected_line.color.g / player.view_distance * abs(ray.total_distance - player.view_distance);
            int b = ray.intersected_line.color.b / player.view_distance * abs(ray.total_distance - player.view_distance);
            draw_line(renderer_gloom, vector<int>{half_width, half_height}, i, height, i, -height, Color(r, g, b, 255 / player.view_distance * abs(ray.total_distance - player.view_distance)));
            if (ray.is_reflecting){
                height = 4000 / (ray.distance * cos(ray.angle_rad - player.angle_rad));
                draw_line(renderer_gloom, vector<int>{half_width, half_height}, i, height, i, -height, Color(0, 0, 255, 50));
            }
        }

        render(renderer);
        render(renderer_gloom);

        wait(13);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    close(window);
    close(window_gloom);

    return 0;
}
