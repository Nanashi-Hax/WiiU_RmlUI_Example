#version 450

in vec2 vUV;
in vec4 vColor;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D Texture;

layout(binding = 1) uniform Time
{
    float iTime;
};

layout(binding = 2) uniform Resolution
{
    vec2 iResolution;
};

mat2 rotate2d(float a)
{
    return mat2(cos(a), -sin(a), sin(a), cos(a));
}

void main()
{
    // UV（0〜1）を画面解像度に変換してピクセル座標にする
    vec2 pixelPos = vUV * iResolution;
    pixelPos.y = iResolution.y - pixelPos.y;

    vec4 accumulatedColor = vec4(0.0);
    
    accumulatedColor *= 3.0;

    for (float i = 1.0; i < 50.0; i += 1.0)
    {
        // 各レイヤーの色（変化するベクトル）
        vec4 layerColor = i * cos(i + vec4(0.0, 2.0, 4.0, 0.0)) + i;
        layerColor *= 0.0003;

        // オーブ（波）の位置に基づく距離で減衰
        float attenuation = length(
            sin(
                (pixelPos / iResolution.y) * 60.0 / i +
                iTime * 0.2 +
                cos(i * vec2(9.0, 7.0))
            )
        );

        accumulatedColor += layerColor * 1.5 / attenuation;
    }

    // 明るさ調整・トーンマッピング
    accumulatedColor = tanh(accumulatedColor * accumulatedColor);

    fragColor = accumulatedColor;

    vec4 textureColor = texture(Texture, vUV);
    fragColor *= textureColor;
}