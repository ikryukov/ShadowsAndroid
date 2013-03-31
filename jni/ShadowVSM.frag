highp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);
varying highp vec4 fPosition;

void main(void)
{
	gl_FragColor = vec4(fPosition.z, fPosition.z * fPosition.z, 0, 0);
}
