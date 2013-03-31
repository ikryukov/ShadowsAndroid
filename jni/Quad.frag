uniform highp sampler2D sShadow;
varying lowp vec4 fColor;
varying lowp vec3 fNormal;
varying lowp vec2 fTexCoord;

void main(void)
{
	gl_FragColor = texture2D(sShadow, fTexCoord);
	//gl_FragColor = vec4(1,0,0,1);
}
