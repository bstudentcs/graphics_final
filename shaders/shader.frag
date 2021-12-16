#version 400 core
out vec4 fragColor;

const int numPlanets = 5;
const float IMPLICIT_SHAPE_RADIUS = 0.2f;

uniform vec2 screenResolution;
uniform vec3 uvk;
uniform vec4 eye;
uniform mat4 inv_p;
uniform mat4 inv_v;
uniform mat4 m[numPlanets];
uniform mat4 inv_m[numPlanets];
uniform vec4 planet_colors[numPlanets];

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

bool intersection(vec4 eye, vec4 d, inout vec4 pos, inout vec4 grad, inout float min_t){
    vec3 new_d = d.xyz;
    vec3 new_eye = eye.xyz;

    //determine point of intersection
    float a = dot(new_d, new_d);
    float b = 2.f*dot(new_eye, new_d);
    float c = dot(new_eye, new_eye) - IMPLICIT_SHAPE_RADIUS*IMPLICIT_SHAPE_RADIUS;
    float sol = smallest_quad_solution(a, b, c);

    //return time, as well as position and gradient
    if ((sol > 0.f && sol < min_t) || min_t < 0.f){
        pos = eye + sol*d;
        grad = normalize(vec4(pos.xyz, 0));
        min_t = sol;
        return true;
    }
    return false;
}

vec4 calc_color(vec4 cDiffuse, vec4 eye, vec4 pos, vec4 grad){
    vec4 cAmbient = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 light_pos = vec4(0, 0, 0, 1);
    vec4 light_color = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 to_light = normalize(pos - light_pos);
    float cos = dot(-grad, to_light);
    vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
    if (cos > 0){
       diffuse = cDiffuse*light_color*cos;
    }
    return diffuse + cAmbient;//vec4(1.0, 1.0, 1.0, 1.0);
}

void main(){
    //set up ray direction
    vec2 uv = (gl_FragCoord.xy + 0.5f)/screenResolution - 0.5f;
    vec4 d = vec4(uv.x*uvk.x, uv.y*uvk.y, -uvk.z, 0.f);
    d = inv_v*normalize(d);
    vec4 camera = inv_v*vec4(0, 0, 0, 1);

    float min_t = -1.f;
    vec4 pos = vec4(0, 0, 0, 1); //point of intersection
    vec4 grad = vec4(0, 0, 0, 0); //the gradient
    vec4 color;

    //check each shape for intersection with ray
    for (int i = 0; i < numPlanets; i++){
        vec4 transformed_d = inv_m[i]*d;
        vec4 transformed_eye = inv_m[i]*camera;
        if (intersection(transformed_eye, transformed_d, pos, grad, min_t)){
            color = planet_colors[i];
            grad = transpose(inv_m[i])*grad;

            //sun is a special case
            if (i == 0){
                grad = -grad;
            }
            pos = m[i]*pos;
        }
    }

    //shade the shapes based on intersections
    if (min_t > 0.f){
        fragColor = calc_color(color, camera, pos, grad);
    } else {
        fragColor = vec4(0.f, 0.f, 0.f, 0.f);
    }
}
