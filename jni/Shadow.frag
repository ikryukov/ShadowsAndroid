uniform highp sampler2D shadowMapTex;

varying highp vec4 fColor;
varying highp vec3 fNormal;
varying highp vec2 fTexCoord;
varying highp vec4 fShadowMapCoord;

highp vec3 Light = vec3(0.0, 4.0, 7.0);
highp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);

void main(void)
{
	const lowp float fAmbient = 0.4;
	Light = normalize(Light);
	highp float depth = (fShadowMapCoord.z / fShadowMapCoord.w);
	highp float depth_light = texture2DProj(shadowMapTex, fShadowMapCoord).r;
	highp float visibility = depth <= depth_light ? 1.0 : 0.2;
	gl_FragColor = fColor * max(0.0, dot(fNormal, Light)) * visibility;
}
