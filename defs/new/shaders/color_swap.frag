uniform sampler2D texture;
uniform vec4 color1;
uniform vec4 replace1;
uniform vec4 color2;
uniform vec4 replace2;

void main()
{
	vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	vec4 eps = vec4(0.009, 0.009, 0.009, 0.009);

	if( all( greaterThanEqual(pixel, color1 - eps) ) && all( lessThanEqual(pixel, color1 + eps) ) )
		pixel = replace1;
	
	if( all( greaterThanEqual(pixel, color2 - eps) ) && all( lessThanEqual(pixel, color2 + eps) ) )
		pixel = replace2;

	gl_FragColor = pixel;
}
