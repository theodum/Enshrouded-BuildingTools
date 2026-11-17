#version 450
// Independent camera-relative line segments, one ribbon instance per segment.
layout(push_constant) uniform Push { mat4 mvp; vec4 size; vec4 eye; vec4 capA; vec4 capB; vec4 cells; } pc;
layout(set=1,binding=0) readonly buffer Lines { float v[]; } lines;
layout(location=0) out vec2 uv;
layout(location=1) flat out vec4 color;
const vec2 quad[6] = vec2[6](vec2(0,-1),vec2(1,-1),vec2(1,1),vec2(0,-1),vec2(1,1),vec2(0,1));
void main() {
    int i = int(pc.size.z) + gl_InstanceIndex * 10;
    vec4 a = pc.mvp * vec4(lines.v[i],lines.v[i+1],lines.v[i+2],1);
    vec4 b = pc.mvp * vec4(lines.v[i+3],lines.v[i+4],lines.v[i+5],1);
    color = vec4(lines.v[i+6],lines.v[i+7],lines.v[i+8],lines.v[i+9]);
    uv = quad[gl_VertexIndex];
    if (a.w < 0.06 && b.w < 0.06) { gl_Position=vec4(0,0,-1,1); return; }
    if (a.w < 0.06) a=mix(a,b,(0.06-a.w)/(b.w-a.w));
    if (b.w < 0.06) b=mix(b,a,(0.06-b.w)/(a.w-b.w));
    vec2 d=(b.xy/b.w-a.xy/a.w)*pc.cells.zw;
    vec2 dir=length(d)>0.0001?normalize(d):vec2(1,0);
    vec2 delta=vec2(-dir.y,dir.x)*pc.size.x*2.0/pc.cells.zw*uv.y;
    vec4 p=uv.x<0.5?a:b;
    gl_Position=vec4((p.xy/p.w+delta)*p.w,p.z,p.w);
}
