#version 410 core
layout(isolines) in;

uniform int numCP;
uniform vec2 cp[20];

float binomial(int n, int k, float t)
{
    int m = n - 1;
    float res = 1.0;
    for(int i = 1; i <= k; i++)
        res = res * float(m - i + 1) / float(i);
    return res * pow(t, float(k)) * pow(1.0 - t, float(m - k));
}

void main()
{
    float t = gl_TessCoord.x;
    vec2 pos = vec2(0.0);
    int n = numCP;

    for(int i = 0; i < n; i++)
        pos += cp[i] * binomial(n, i, t);

    gl_Position = vec4(pos, 0.0, 1.0);
}