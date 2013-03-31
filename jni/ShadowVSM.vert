attribute vec3 Position;

varying vec4 fPosition;

uniform mat4 Projection;
uniform mat4 Modelview;

void main(void)
{
	gl_Position = Projection * Modelview * vec4(Position, 1);
	fPosition = gl_Position;
}
