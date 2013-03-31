attribute vec3 Position;
attribute vec3 Normal;
attribute vec4 SourceColor;
attribute vec2 TexCoord;

varying vec4 fColor;
varying vec3 fNormal;
varying vec2 fTexCoord;
varying vec4 fShadowMapCoord;

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat4 lightMatrix;
uniform mat4 Transform;

void main(void)
{
	fColor = SourceColor;
	gl_Position = Projection * Modelview * Transform * vec4(Position, 1.0);
	fShadowMapCoord = lightMatrix * Transform * vec4(Position, 1.0);
	fNormal = normalize(Normal);
	fTexCoord = TexCoord;
}
