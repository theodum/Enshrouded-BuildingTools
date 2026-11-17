#version 450
layout(location=0) in vec2 uv;
layout(location=1) flat in vec4 color;
layout(location=0) out vec4 result;
void main() { result=vec4(color.rgb,color.a*(1.0-smoothstep(0.6,1.0,abs(uv.y)))); }
