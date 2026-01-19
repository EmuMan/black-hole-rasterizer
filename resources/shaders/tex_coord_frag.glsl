#version 330 core

in vec2 vTexCoord;
out vec4 color;

void main() {
	color = vec4(vTexCoord.s, vTexCoord.t, vTexCoord.s, 1);
}
