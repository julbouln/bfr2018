uniform sampler2D texture;
uniform float amount;

void main()
{
	float d = 1. / amount;
 vec2 coord = vec2(d*floor(gl_TexCoord[0].x/d),
                   d*floor(gl_TexCoord[0].y/d));
 gl_FragColor = texture2D(texture, coord);
}
