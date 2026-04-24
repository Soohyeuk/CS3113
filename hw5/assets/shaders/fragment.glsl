#version 330

const float RED_LUM_CONSTANT = 0.2126;
const float GREEN_LUM_CONSTANT = 0.7152;
const float BLUE_LUM_CONSTANT = 0.0722;

uniform sampler2D texture0;
uniform vec2 lightPosition;

uniform float uTime;
uniform float uEmergency;

in vec2 fragTexCoord;
in vec2 fragPosition;

out vec4 finalColor;

// Adjustable attenuation parameters
const float LINEAR_TERM    = 0.00006; // linear term
const float QUADRATIC_TERM = 0.00045; // quadratic term  (larger = smaller disc)
const float DARK_CUTOFF    = 0.20;    // attenuation values below this -> pure black

float attenuate(float distance, float linearTerm, float quadraticTerm)
{
    return 1.0 / (1.0 +
                  linearTerm * distance +
                  quadraticTerm * distance * distance);
}

void main()
{
    float distance = distance(lightPosition, fragPosition);
    float brightness = attenuate(distance, LINEAR_TERM, QUADRATIC_TERM);
    // Remap [DARK_CUTOFF, 1.0] -> [0.0, 1.0] so the dim tail becomes solid black
    // while the visible disc keeps roughly the same size and smooth edge.
    brightness = max((brightness - DARK_CUTOFF) / (1.0 - DARK_CUTOFF), 0.0);

    vec4 color = texture(texture0, fragTexCoord);

    vec3 rgb = color.rgb * brightness;

    if (uEmergency > 0.5)
    {
        float pulse = 0.5 + 0.5 * sin(uTime * 10.0);
        vec3 red = vec3(1.0, 0.12, 0.12);
        rgb = mix(rgb, red, pulse * 0.55);
    }

    finalColor = vec4(rgb, color.a);
}
