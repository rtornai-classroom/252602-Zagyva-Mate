#version 330

in vec3         varyingFragmentPosition;
in vec3         varyingNormal;

uniform vec3    lightPosition;
uniform vec3    cameraPosition;
uniform vec3    lightColor;         // diffuse light szine
uniform bool    lightingEnabled;

out vec4        outColor;

void main(void) {
    // White material for the cubes
    const vec3 materialColor = vec3(1.0, 1.0, 1.0);

    if (!lightingEnabled) {
        outColor = vec4(materialColor, 1.0);
        return;
    }

    vec3 normalizedNormal   = normalize(varyingNormal);
    vec3 lightDirection     = normalize(lightPosition - varyingFragmentPosition);
    vec3 viewDirection      = normalize(cameraPosition - varyingFragmentPosition);
    vec3 reflectDirection   = reflect(-lightDirection, normalizedNormal);

    // AMBIENT
    float ambientStrength   = 0.2;
    vec3  ambient           = ambientStrength * lightColor * materialColor;

    // DIFFUSE
    float diff              = max(dot(normalizedNormal, lightDirection), 0.0);
    vec3  diffuse           = diff * lightColor * materialColor;

    // SPECULAR
    float specularStrength  = 0.5;
    int   shininess         = 32;
    float spec              = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
    vec3  specular          = specularStrength * spec * lightColor;

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
