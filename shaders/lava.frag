#version 120

uniform sampler2D uTexture;
uniform float uTime;       // tempo em segundos
uniform float uStrength;   // força da distorção (use 1.0)
uniform vec2  uScroll;     // direção/velocidade do fluxo
uniform float uHeat;       // vamos reutilizar como "brilho"/intensidade

varying vec2 vTexCoord;

float hash(vec2 p) {
    // ruído baratinho (determinístico)
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main()
{
    // fluxo base
    vec2 uv = vTexCoord + uScroll * uTime;

    // ondas: mistura de sen/cos em 2 direções
    float w1 = sin(uv.x * 12.0 + uTime * 1.4);
    float w2 = cos(uv.y * 10.0 + uTime * 1.1);
    float w3 = sin((uv.x + uv.y) * 7.0 + uTime * 0.8);

    // distorção suave
    vec2 wave = vec2(w1 + 0.35*w3, w2 - 0.35*w3);
    vec2 uvDistorted = uv + uStrength * wave * 0.015;

    // amostra textura (sua textura atual vira "base" da água)
    vec4 baseColor = texture2D(uTexture, uvDistorted);

    // cor da água (templo/ruínas: verde-azulado)
    vec3 waterTint = vec3(0.10, 0.55, 0.45);

    // mistura com a textura
    vec3 col = mix(baseColor.rgb, waterTint, 0.55);

    // "reflexo" fake (highlight que corre)
    float ripple = (w1*w2);
    float spec = pow(abs(ripple), 3.0);

    // micro brilho aleatório sutil
    float n = hash(floor(uv * 40.0) + uTime * 0.5);
    float sparkle = smoothstep(0.92, 1.0, n) * 0.15;

    float shine = (spec * 0.9 + sparkle) * (0.6 + uHeat * 0.8);

    col += vec3(0.25, 0.45, 0.45) * shine;

    // leve escurecimento pra profundidade
    col *= 0.92;

    gl_FragColor = vec4(clamp(col, 0.0, 1.0), baseColor.a);
}