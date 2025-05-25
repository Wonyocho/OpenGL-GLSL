// File: shader.frag
#version 120
varying vec3 Normal;
varying vec3 FragPos;
uniform vec3 lightDir;
uniform vec3 objectColor;

void main() {
    float diff = max(dot(normalize(Normal), normalize(lightDir)), 0.0);
    vec3 diffuse = diff * objectColor;
    gl_FragColor = vec4(diffuse, 1.0);
}
