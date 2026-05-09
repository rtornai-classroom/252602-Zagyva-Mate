#version 330 core

in vec3 varyingFragmentPosition;
in vec3 varyingNormal;
in vec2 varyingTextureCoord;

out vec4 outColor;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 objectColor;
uniform bool useLighting;
uniform bool useTexture;
uniform sampler2D textureSampler;

const vec3 ambientColor = vec3(1.0, 1.0, 1.0);
const float ambientStrength = 0.2;
const vec3 diffuseLightColor = vec3(1.0, 0.9, 0.5);

void main(void)
{
  if (useTexture)
  {
    // Nap rajzolas es textura
    outColor = texture(textureSampler, varyingTextureCoord);
  }
  else
  {
    vec3 ambient = ambientStrength * ambientColor;

    if (useLighting)
    {
      vec3 norm = normalize(varyingNormal);
      vec3 lightDir = normalize(lightPosition - varyingFragmentPosition);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * diffuseLightColor;

      // Feher szin atadasa
      outColor = vec4((ambient + diffuse) * objectColor, 1.0);
    }
    else
    {
      outColor = vec4(ambient * objectColor, 1.0);
    }
  }
}
