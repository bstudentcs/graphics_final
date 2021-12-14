#version 400 core
out vec4 fragColor;

const int numPlanets = 5;

uniform vec2 screenResolution;
uniform mat4 p;
uniform mat4 v;
uniform mat4 m[numPlanets];

void main(){
    vec2 uv = gl_FragCoord.xy/screenResolution;
    if (length(uv - 0.5) < 0.1){
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
}
