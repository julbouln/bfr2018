const float offset = 1.0 / 512.0;
uniform sampler2D texture;

void main()
{
	vec2 texturePos = gl_TexCoord[0].xy;
	float alpha = 4.0*texture2D( texture, texturePos ).a;
    alpha -= texture2D( texture, texturePos + vec2( offset, 0.0f ) ).a;
    alpha -= texture2D( texture, texturePos + vec2( -offset, 0.0f ) ).a;
    alpha -= texture2D( texture, texturePos + vec2( 0.0f, offset ) ).a;
    alpha -= texture2D( texture, texturePos + vec2( 0.0f, -offset ) ).a;
 	gl_FragColor = vec4( 1.0f, 1.0f, 1.0f, alpha );
 }