// Interpolated values from the vertex shaders
varying vec2 UV;
varying vec3 Position_worldspace;
varying vec3 Normal_cameraspace;
varying vec3 EyeDirection_cameraspace;
varying vec3 LightDirection_cameraspace;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform mat4 MV;
uniform vec3 LightPosition_worldspace;

float threshold( float thr1,  float thr2 ,  float val) {
 if (val < thr1) {return 0.0;}
 if (val > thr2) {return 1.0;}
 return val;
}

// averaged pixel intensity from 3 color channels
float avg_intensity( vec4 pix) {
 return (pix.r + pix.g + pix.b)/3.;
}

vec4 get_pixel( vec2 coords,  float dx,  float dy) {
 return texture2D(myTextureSampler, coords + vec2(dx, dy));
}

// returns pixel color
float IsEdge( vec2 coords){
  float dxtex = 1.0 / 1280.0 /*image width*/;
  float dytex = 1.0 / 960.0 /*image height*/;
  float pix[9];
  int k = -1;
  float delta;

  // read neighboring pixel intensities
  /*
  for (int i=-1; i<2; i++) {
   for(int j=-1; j<2; j++) {
    k++;
    pix[k] = avg_intensity(get_pixel(coords,float(i)*dxtex,
                                          float(j)*dytex));
   }
  }
  */
  
    pix[0] = avg_intensity(get_pixel(coords,float(-1)*dxtex,
                                          float(-1)*dytex));
    pix[1] = avg_intensity(get_pixel(coords,float(-1)*dxtex,
                                          float(0)*dytex));
    pix[2] = avg_intensity(get_pixel(coords,float(-1)*dxtex,
                                          float(1)*dytex));
                                          
    pix[3] = avg_intensity(get_pixel(coords,float(0)*dxtex,
                                          float(-1)*dytex));
    pix[4] = avg_intensity(get_pixel(coords,float(0)*dxtex,
                                          float(0)*dytex));
    pix[5] = avg_intensity(get_pixel(coords,float(0)*dxtex,
                                          float(1)*dytex));
                                          
    pix[6] = avg_intensity(get_pixel(coords,float(1)*dxtex,
                                          float(-1)*dytex));
    pix[7] = avg_intensity(get_pixel(coords,float(1)*dxtex,
                                          float(0)*dytex));
    pix[8] = avg_intensity(get_pixel(coords,float(1)*dxtex,
                                          float(1)*dytex));
  

  // average color differences around neighboring pixels
  delta = (abs(pix[1]-pix[7])+
          abs(pix[5]-pix[3]) +
          abs(pix[0]-pix[8])+
          abs(pix[2]-pix[6])
           )/4.;

  return threshold(0.25,0.4,clamp(1.8*delta,0.0,1.0));
}

void main(){

// Light emission properties
// You probably want to put them as uniforms
vec3 LightColor = vec3(1,1,1);
float LightPower = 50.0;

// Material properties
vec3 MaterialDiffuseColor = texture2D( myTextureSampler, UV ).rgb;
vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

// Distance to the light
float distance = length( LightPosition_worldspace - Position_worldspace );

// Normal of the computed fragment, in camera space
vec3 n = normalize( Normal_cameraspace );
// Direction of the light (from the fragment to the light)
vec3 l = normalize( LightDirection_cameraspace );
// Cosine of the angle between the normal and the light direction,
// clamped above 0
// - light is at the vertical of the triangle -> 1
// - light is perpendicular to the triangle -> 0
// - light is behind the triangle -> 0
float cosTheta = clamp( dot( n,l ), 0.0, 1.0 );

// Eye vector (towards the camera)
vec3 E = normalize(EyeDirection_cameraspace);
// Direction in which the triangle reflects the light
vec3 R = reflect(-l,n);
// Cosine of the angle between the Eye vector and the Reflect vector,
// clamped to 0
// - Looking into the reflection -> 1
// - Looking elsewhere -> < 1
float cosAlpha = clamp( dot( E,R ), 0.0, 1.0 );

vec3 amb = MaterialAmbientColor;
vec3 dif = MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance);
vec3 spec = MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5.0) / (distance*distance);

gl_FragColor.a = 1.0;
gl_FragColor.r = IsEdge(UV);
//gl_FragColor.rgb = amb + dif + spec;
/*if (IsEdge(UV) > 0) {
 gl_FragColor.rgb = MaterialDiffuseColor;
}*/
/*if (MaterialDiffuseColor.r > 0.3) {*/
 /*gl_FragColor.rgb = vec3(1.0, 1.0, 1.0);*/
 
/*}*/

/* gl_FragColor.rgb =
// Ambient : simulates indirect lighting
MaterialAmbientColor +
// Diffuse : "color" of the object
MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
// Specular : reflective highlight, like a mirror
MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
*/
}
