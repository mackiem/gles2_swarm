// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../common/startScreen.h"
#include "../common/LoadShaders.h"
#include "../common/texture.h"
#include "../common/objloader.h"

#include "raspicam_cv.h"
#include <iostream>
#include <opencv2/opencv.hpp>

#include "raspicam.h"
#include <string>
#include <fstream>

//raspicam::RaspiCam_Cv camera_;
raspicam::RaspiCam camera_;

GLuint init_texture(int width, int height) {

    //Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); //GL_BGR barfs


    // Poor filtering, or ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // ... nice trilinear filtering.
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glGenerateMipmap(GL_TEXTURE_2D);

    printf("Returning textureID %d\n", textureID);

    // Return the ID of the texture we just created
    return textureID;


}

unsigned char* image;

void saveImage ( std::string filepath,unsigned char *data,raspicam::RaspiCam &Camera  ) {
        std::ofstream outFile ( filepath.c_str(),std::ios::binary  );
        if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_BGR ||  Camera.getFormat()==raspicam::RASPICAM_FORMAT_RGB  ) {
                    outFile<<"P6\n";

        } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_GRAY  ) {
                    outFile<<"P5\n";

        } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_YUV420  ) { //made up format
                outFile<<"P7\n";
            }
            outFile<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
                outFile.write ( ( char*  ) data,Camera.getImageBufferSize()  );

}


void init_camera(int width, int height) {

    //camera_.set ( CV_CAP_PROP_FRAME_WIDTH,  width);
    //camera_.set ( CV_CAP_PROP_FRAME_HEIGHT, height);
    //camera_.set ( CV_CAP_PROP_BRIGHTNESS, 50  );
    //camera_.set ( CV_CAP_PROP_CONTRAST, 50  ) ;
    //camera_.set ( CV_CAP_PROP_SATURATION, 50  );
    //camera_.set ( CV_CAP_PROP_GAIN, 50);
    //

    camera_.setWidth(width);
    camera_.setHeight(height);
    camera_.setBrightness(50);
    camera_.setSharpness(0);
    camera_.setContrast(0);
    camera_.setSaturation(0);
    camera_.setShutterSpeed(0);
    camera_.setISO(400);
    camera_.setExposureCompensation(0);


    //camera_.setFormat(raspicam::RASPICAM_FORMAT_GRAY);

    if ( !camera_.open() ) {
        std::cerr << "Error opening camera" << std::endl;
    }
    std::cout<<"Connected to camera ="<<camera_.getId() <<" bufs="<<camera_.getImageBufferSize() << std::endl;

    image = new unsigned char[ camera_.getImageBufferSize() ];
}

void set_camera_to_texture(GLuint tex_id, int width, int height) {
    //cv::Mat image;
    camera_.grab();
    camera_.retrieve(image);
    //cv::cvtColor(image, image, CV_BGR2RGB);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, tex_id);

    //std::cout << tex_id << " " << width << " " << height << std::endl;
    // Give the image to OpenGL
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.ptr()); //GL_BGR barfs
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); //GL_BGR barfs
    //cv::imwrite("image.jpg", image);
    //saveImage("image00.ppm", image, camera_);
}

int main( void )
{
InitGraphics();
    printf("Screen started\n");

// Dark blue background
// glClearColor(0.0f, 0.0f, 0.4f, 1.0f);

// Enable depth test
glEnable(GL_DEPTH_TEST);
// Accept fragment if it closer to the camera than the former one
glDepthFunc(GL_LESS);

// Cull triangles which normal is not towards the camera
//glEnable(GL_CULL_FACE);

// Create and compile our GLSL program from the shaders
GLuint programID = LoadShaders( "TransformVertexShader.glsl", "TextureFragmentShader.glsl" );

// Get a handle for our "MVP" uniform
GLuint MatrixID = glGetUniformLocation(programID, "MVP");
GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

// Get a handle for our buffers
GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    glm::mat4 View = glm::lookAt(
                                glm::vec3(0,0,-2.5), // Camera is at (4,1,3), in World Space
                                glm::vec3(0,0,0), // and looks at the origin
                                glm::vec3(0,1,0) // Head is up (set to 0,-1,0 to look upside-down)
                           );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Transform for Model
    glm::vec3 position = glm::vec3(0,0,0);
    glm::vec3 rotation = glm::vec3(0,0,0);
    glm::vec3 scale = glm::vec3(1,1,1);


    // init camera
    int width = 1280;
    int height = 960;

    init_camera(width, height);
    GLuint Texture = init_texture(width, height);

// Load the texture
//GLuint Texture = loadBMP_custom("uvtemplate.bmp");

// Get a handle for our "myTextureSampler" uniform
GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

// Read our .obj file
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;
//bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
bool res = loadOBJ("cube-textures.obj", vertices, uvs, normals);

// Load it into a VBO
GLuint vertexbuffer;
glGenBuffers(1, &vertexbuffer);
glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

GLuint uvbuffer;
glGenBuffers(1, &uvbuffer);
glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

GLuint normalbuffer;
glGenBuffers(1, &normalbuffer);
glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

// Get a handle for our "LightPosition" uniform
glUseProgram(programID);
GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

do{
// Clear the screen
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Use our shader
glUseProgram(programID);

        // Rebuild the Model matrix
        //rotation.y += 0.01f;
glm::mat4 translationMatrix	= glm::translate(glm::mat4(1.0f), position);
glm::mat4 rotationMatrix	= glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
glm::mat4 scalingMatrix	= glm::scale(glm::mat4(1.0f), scale);

Model = translationMatrix * rotationMatrix * scalingMatrix;

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

// Send our transformation to the currently bound shader,
// in the "MVP" uniform
glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

glm::vec3 lightPos = glm::vec3(4,4,4);
glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

// Bind our texture in Texture Unit 0
glActiveTexture(GL_TEXTURE0);
//glBindTexture(GL_TEXTURE_2D, Texture);
set_camera_to_texture(Texture, width, height);

// Set our "myTextureSampler" sampler to user Texture Unit 0
glUniform1i(TextureID, 0);

// 1rst attribute buffer : vertices
glEnableVertexAttribArray(vertexPosition_modelspaceID);
glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
glVertexAttribPointer(
vertexPosition_modelspaceID, // The attribute we want to configure
3, // size
GL_FLOAT, // type
GL_FALSE, // normalized?
0, // stride
(void*)0 // array buffer offset
);

// 2nd attribute buffer : UVs
glEnableVertexAttribArray(vertexUVID);
glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
glVertexAttribPointer(
vertexUVID, // The attribute we want to configure
2, // size : U+V => 2
GL_FLOAT, // type
GL_FALSE, // normalized?
0, // stride
(void*)0 // array buffer offset
);

// 3rd attribute buffer : normals
glEnableVertexAttribArray(vertexNormal_modelspaceID);
glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
glVertexAttribPointer(
vertexNormal_modelspaceID, // The attribute we want to configure
3, // size
GL_FLOAT, // type
GL_FALSE, // normalized?
0, // stride
(void*)0 // array buffer offset
);

// Draw the triangles !
glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

glDisableVertexAttribArray(vertexPosition_modelspaceID);
glDisableVertexAttribArray(vertexUVID);
glDisableVertexAttribArray(vertexNormal_modelspaceID);

updateScreen();
}
while( 1 );

delete image;
// Cleanup VBO and shader
glDeleteBuffers(1, &vertexbuffer);
glDeleteBuffers(1, &uvbuffer);
glDeleteBuffers(1, &normalbuffer);
glDeleteProgram(programID);
glDeleteTextures(1, &Texture);

return 0;
}
