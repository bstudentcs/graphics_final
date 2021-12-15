#version 400 core
out vec4 fragColor;

const int numPlanets = 5;
const float IMPLICIT_SHAPE_RADIUS = 0.2f;

uniform vec2 screenResolution;
uniform vec3 uvk;
uniform vec4 eye;
uniform mat4 inv_p;
uniform mat4 inv_v;
uniform mat4 inv_m[numPlanets];

float smallest_quad_solution(float a, float b, float c){
    float discriminant_sqrd = b*b - 4*a*c;
    //no real solution, return a negative one
    if (discriminant_sqrd < 0.f){
        return -1.f;
    }

    //return smallest positive solution, if there is one
    float discriminant = sqrt(discriminant_sqrd);
    if (b + discriminant < 0.f) {
        return (-b - discriminant)/(2.f*a);
    }
    return (-b + discriminant)/(2.f*a);
}

float intersection(vec3 eye, vec3 d){
    float a = dot(d, d);
    float b = 2.f*dot(eye, d);
    float c = dot(eye, eye) - IMPLICIT_SHAPE_RADIUS*IMPLICIT_SHAPE_RADIUS;
    float sol = smallest_quad_solution(a, b, c);
    return sol;
}

void main(){
    //set up ray direction
    vec2 uv = (gl_FragCoord.xy + 0.5f)/screenResolution - 0.5f;
    vec4 d = vec4(uv.x*uvk.x, uv.y*uvk.y, -uvk.z, 0.f);
    d = inv_v*normalize(d);
    vec4 camera = inv_v*eye;

    float min_t = -1.f;
    for (int i = 0; i < numPlanets; i++){
        vec4 transformed_d = inv_m[i]*d;
        vec4 transformed_eye = inv_m[i]*camera;
        float sol = intersection(transformed_eye.xyz, transformed_d.xyz);
        if ((sol > 0.f && sol < min_t) || min_t < 0.f){
            min_t = sol;
        }
    }
    if (min_t > 0.f){
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.f, 0.f, 0.f, 0.f);
    }
}
