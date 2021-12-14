#version 400 core
out vec4 fragColor;

const int numPlanets = 5;
const float IMPLICIT_SHAPE_RADIUS = 0.2f;

uniform vec2 screenResolution;
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

float intersection(vec4 eye, vec4 d){
    float a = dot(d, d);
    float b = 2.f*dot(eye, d);
    float c = dot(eye, eye) - IMPLICIT_SHAPE_RADIUS*IMPLICIT_SHAPE_RADIUS;
    float sol = smallest_quad_solution(a, b, c);
    return sol;
}

void main(){
    vec2 uv = gl_FragCoord.xy/screenResolution;
    uv = (uv - 0.5)*2.f;
    vec4 d = normalize(inv_v*inv_p*vec4(uv, 1.f, 0.f));
    vec4 camera = inv_v*inv_p*eye;

    float min_t = -1.f;
    for (int i = 0; i < numPlanets; i++){
        vec4 transformed_d = inv_m[i]*d;
        vec4 transformed_eye = inv_m[i]*eye;
        float sol = intersection(transformed_eye, transformed_d);
        if ((sol > 0.f && sol < min_t) || min_t < 0.f){
            min_t = sol;
        }
    }
    /*if (length(uv - 0.5) < 0.1){
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }*/
    if (min_t > 0.f){
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.f, 0.f, 1.f, 0.f);//vec4(dot(d, vec4(-1.f, 0.f, 0.f, 0.f)));
    }
}
