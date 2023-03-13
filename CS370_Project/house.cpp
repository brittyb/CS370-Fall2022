// CS370 Final Project
// Fall 2022

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#include "../common/tangentspace.h"

#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Table, Chair, Door, Cup, Soda, Circle, Bowl, Sphere, Blinds, Fan, Frame, Drawer, TV, Plane, Painting, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, TangBuffer, BiTangBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube, BlueCube, GreenCube, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Walls, CupMaterial, WhiteMaterial, SodaMaterial, TVMaterial, DresserMaterial};
enum Textures {Wood, Carpet, Apple, Popeye, Window, SodaTex, SodaTop, Wednesday, Splatoon, Coyote, FruitNorm, WoodNorm, MirrorTex, NumTextures};


// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];


// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;
GLint tangCoords = 3;
GLint bitangCoords = 3;

// Model files
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/table.obj", "../models/chair.obj", "../models/door.obj", "../models/cup.obj",
                                 "../models/cylinder.obj", "../models/circle.obj", "../models/bowl.obj", "../models/sphere.obj", "../models/blinds.obj", "../models/fan.obj",
                                 "../models/frame.obj", "../models/drawer.obj", "../models/tv.obj", "../models/plane.obj"};

// Texture files
vector<const char *> texFiles = {"../textures/wood.jpg", "../textures/carpet.jpg",  "../textures/apple.jpg", "../textures/popeye.png",
                                 "../textures/snow.jpg", "../textures/sodatex.jpg", "../textures/sodatop.jpeg", "../textures/wednesday.jpg",
                                 "../textures/splatoon.jpg", "../textures/wileecoyote.jpg", "../textures/fruitnorm.jpg", "../textures/woodnorm.jpg"};





vec3 mirror_eye = {0.0f, -0.50f, 3.94f};
vec3 mirror_center = {0.0f, -0.50f, 0.0f};
vec3 mirror_up = {0.0f, 1.0f, 0.0f};

GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;

// Camera
vec3 eye = {0.0f, 0.0f, 0.0f };
vec3 center = {5.6f, -2.5f, 1.0f};
vec3 up = {0.0f, 1.0f, 0.0f};



// Shader variables
// Default (color) shader program references
GLuint default_vPos;
GLuint default_program;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";


// Light shader program with shadows reference
GLuint phong_shadow_program;
GLuint phong_shadow_vPos;
GLuint phong_shadow_vNorm;
GLuint phong_shadow_proj_mat_loc;
GLuint phong_shadow_camera_mat_loc;
GLuint phong_shadow_norm_mat_loc;
GLuint phong_shadow_model_mat_loc;
GLuint phong_shadow_shad_proj_mat_loc;
GLuint phong_shadow_shad_cam_mat_loc;
GLuint phong_shadow_lights_block_idx;
GLuint phong_shadow_materials_block_idx;
GLuint phong_shadow_material_loc;
GLuint phong_shadow_num_lights_loc;
GLuint phong_shadow_light_on_loc;
GLuint phong_shadow_eye_loc;
const char *phong_shadow_vertex_shader = "../phongShadow.vert";
const char *phong_shadow_frag_shader = "../phongShadow.frag";



// Multi-texture shader program reference
GLuint multi_tex_program;
// Multi-texture shader component references
GLuint multi_tex_vPos;
GLuint multi_tex_vTex;
GLuint multi_tex_proj_mat_loc;
GLuint multi_tex_camera_mat_loc;
GLuint multi_tex_model_mat_loc;
GLuint multi_tex_base_loc;
GLuint multi_tex_dirt_loc;
const char *multi_tex_vertex_shader = "../multiTex.vert";
const char *multi_tex_frag_shader = "../multiTex.frag";

// Bumpmapping shader program reference
GLuint bump_program;
GLuint bump_proj_mat_loc;
GLuint bump_camera_mat_loc;
GLuint bump_norm_mat_loc;
GLuint bump_model_mat_loc;
GLuint bump_vPos;
GLuint bump_vNorm;
GLuint bump_vTex;
GLuint bump_vTang;
GLuint bump_vBiTang;
GLuint bump_lights_block_idx;
GLuint bump_num_lights_loc;
GLuint bump_light_on_loc;
GLuint bump_eye_loc;
GLuint bump_base_loc;
GLuint bump_norm_loc;
const char *bump_vertex_shader = "../bumpTex.vert";
const char *bump_frag_shader = "../bumpTex.frag";

// Debug mirror shader
GLuint debug_mirror_program;
const char *debug_mirror_vertex_shader = "../debugMirror.vert";
const char *debug_mirror_frag_shader = "../debugMirror.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Generic shader variables references
GLuint vPos;
GLuint vNorm;
GLuint model_mat_loc;

// Mirror flag
GLboolean mirror = false;

// Shadow flag
GLuint shadow = false;

//animation variables
GLboolean fan = false;
GLfloat fan_angle = 0.0f;

GLboolean blinds = false;
GLfloat blinds_scale = 1.1f;
GLfloat blinds_dir = 1.0f;
// Rotation angles
GLdouble elTime = 0.0;
GLfloat theta = 0.0f;
GLfloat rpm = 2.5f;

GLint channel = 0;

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;
mat4 shadow_proj_matrix;
mat4 shadow_camera_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );

void build_textures();

void build_mirror();
void create_mirror();
void build_painting();
void load_bump_object(GLuint obj);
void draw_bump_object(GLuint obj, GLuint base_texture, GLuint normal_map);
void draw_multi_tex_object(GLuint obj, GLuint texture1, GLuint texture2);
void renderQuad(GLuint shader, GLuint tex);

void load_object(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_mat_shadow_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void draw_tex_object2(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);



    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();

    build_mirror();

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");


    // Load shaders
    // Load light shader with shadows
    ShaderInfo phong_shadow_shaders[] = { {GL_VERTEX_SHADER, phong_shadow_vertex_shader},{GL_FRAGMENT_SHADER, phong_shadow_frag_shader},{GL_NONE, NULL} };
    phong_shadow_program = LoadShaders(phong_shadow_shaders);
    phong_shadow_vPos = glGetAttribLocation(phong_shadow_program, "vPosition");
    phong_shadow_vNorm = glGetAttribLocation(phong_shadow_program, "vNormal");
    phong_shadow_camera_mat_loc = glGetUniformLocation(phong_shadow_program, "camera_matrix");
    phong_shadow_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "proj_matrix");
    phong_shadow_norm_mat_loc = glGetUniformLocation(phong_shadow_program, "normal_matrix");
    phong_shadow_model_mat_loc = glGetUniformLocation(phong_shadow_program, "model_matrix");
    phong_shadow_shad_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "light_proj_matrix");
    phong_shadow_shad_cam_mat_loc = glGetUniformLocation(phong_shadow_program, "light_cam_matrix");
    phong_shadow_lights_block_idx = glGetUniformBlockIndex(phong_shadow_program, "LightBuffer");
    phong_shadow_materials_block_idx = glGetUniformBlockIndex(phong_shadow_program, "MaterialBuffer");
    phong_shadow_material_loc = glGetUniformLocation(phong_shadow_program, "Material");
    phong_shadow_num_lights_loc = glGetUniformLocation(phong_shadow_program, "NumLights");
    phong_shadow_light_on_loc = glGetUniformLocation(phong_shadow_program, "LightOn");
    phong_shadow_eye_loc = glGetUniformLocation(phong_shadow_program, "EyePosition");


    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Load texture shaders
    ShaderInfo multi_tex_shaders[] = { {GL_VERTEX_SHADER, multi_tex_vertex_shader},{GL_FRAGMENT_SHADER, multi_tex_frag_shader},{GL_NONE, NULL} };
    multi_tex_program = LoadShaders(multi_tex_shaders);
    multi_tex_vPos = glGetAttribLocation(multi_tex_program, "vPosition");
    multi_tex_vTex = glGetAttribLocation(multi_tex_program, "vTexCoord");
    multi_tex_proj_mat_loc = glGetUniformLocation(multi_tex_program, "proj_matrix");
    multi_tex_camera_mat_loc = glGetUniformLocation(multi_tex_program, "camera_matrix");
    multi_tex_model_mat_loc = glGetUniformLocation(multi_tex_program, "model_matrix");
    multi_tex_base_loc = glGetUniformLocation(multi_tex_program, "baseMap");
    multi_tex_dirt_loc = glGetUniformLocation(multi_tex_program, "dirtMap");

    // Load bump shader
    ShaderInfo bump_shaders[] = { {GL_VERTEX_SHADER, bump_vertex_shader},{GL_FRAGMENT_SHADER, bump_frag_shader},{GL_NONE, NULL} };
    bump_program = LoadShaders(bump_shaders);
    bump_vPos = glGetAttribLocation(bump_program, "vPosition");
    bump_vNorm = glGetAttribLocation(bump_program, "vNormal");
    bump_vTex = glGetAttribLocation(bump_program, "vTexCoord");
    bump_vTang = glGetAttribLocation(bump_program, "vTangent");
    bump_vBiTang = glGetAttribLocation(bump_program, "vBiTangent");
    bump_proj_mat_loc = glGetUniformLocation(bump_program, "proj_matrix");
    bump_camera_mat_loc = glGetUniformLocation(bump_program, "camera_matrix");
    bump_norm_mat_loc = glGetUniformLocation(bump_program, "normal_matrix");
    bump_model_mat_loc = glGetUniformLocation(bump_program, "model_matrix");
    bump_lights_block_idx = glGetUniformBlockIndex(bump_program, "LightBuffer");
    bump_num_lights_loc = glGetUniformLocation(bump_program, "NumLights");
    bump_light_on_loc = glGetUniformLocation(bump_program, "LightOn");
    bump_eye_loc = glGetUniformLocation(bump_program, "EyePosition");
    bump_base_loc = glGetUniformLocation(bump_program, "baseMap");
    bump_norm_loc = glGetUniformLocation(bump_program, "normalMap");

    // Load debug mirror shader
    ShaderInfo debug_mirror_shaders[] = { {GL_VERTEX_SHADER, debug_mirror_vertex_shader},{GL_FRAGMENT_SHADER, debug_mirror_frag_shader},{GL_NONE, NULL} };
    debug_mirror_program = LoadShaders(debug_mirror_shaders);


    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = -3.0f;
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        create_mirror();
        //renderQuad(debug_mirror_program, MirrorTex);
        display();
        // Update other events like input handling
        glfwPollEvents();
        GLdouble curTime = glfwGetTime();
        double dT = (curTime-elTime);
        if(fan){
            fan_angle += rpm * dT * 60.0f;
        }
        if(blinds){
            if(blinds_dir == 1.0f){
                blinds_scale -= 0.01*dT*60.0f;
                if(blinds_scale <= 0.15f){
                    blinds_scale = 0.151f;
                    blinds = false;
                    blinds_dir = 0.0f;
                }
            }else{
                blinds_scale += 0.01*dT*60.0f;
                if(blinds_scale >= 1.1f){
                    blinds = false;
                    blinds_dir = 1.0f;
                }
            }

        }
        elTime = curTime;
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // DEFAULT ORTHOGRAPHIC PROJECTION
    //proj_matrix = ortho(-5.0f*xratio, 5.0f*xratio, -5.0f*yratio, 5.0f*yratio, -10.0f, 10.0f);


    proj_matrix = frustum(-0.4f*xratio, 0.4f*xratio, -0.40f*yratio, 0.40f * yratio, 1.0f, 100.00f);

    // TODO: Set camera matrix


    camera_matrix = lookat(eye, center, up);


    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void create_mirror( ){
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    proj_matrix = frustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 100.0f);

    camera_matrix = lookat(mirror_eye, mirror_center, mirror_up);

// Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);
}
void build_mirror( ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[MirrorTex]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 rot2_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();



    //floor
    trans_matrix = translate(0.0f, -4.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(4.0f, 0.1f, 4.0f);
	model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_tex_object(Painting, Carpet);

    //ceiling
    trans_matrix = translate(0.0f, 3.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 0.1f, 8.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Cube, Walls);


    //walls
    trans_matrix = translate(-4.0f, 0.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 8.0f, 8.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Cube, Walls);

    trans_matrix = translate(4.0f, 0.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 8.0f, 8.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Cube, Walls);

    trans_matrix = translate(.0f, 0.0f, -4.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 8.0f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Cube, Walls);


    trans_matrix = translate(.0f, 0.0f, 4.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 8.0f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Cube, Walls);



    //Soda Can
    trans_matrix = translate(-0.4f, -3.1f, 0.1f);
    rot_matrix = rotate(0.0f, vec3(1.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.07f, 0.08f, 0.07f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    draw_tex_object(Soda, SodaTex);

    //Soda Can top
    trans_matrix = translate(-0.40f, -3.01f, 0.095f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.07f, 0.07f, 0.07f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    draw_tex_object(Circle, SodaTop);

    //Dresser
    trans_matrix = translate(-2.45f, -3.6f, -3.395f);
    rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.6f, 0.4f, 0.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(Drawer, DresserMaterial);


    //TV
    trans_matrix = translate(-2.45f, -1.7f, -3.495f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));

    scale_matrix = scale(0.45f, 0.45f, 0.45f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_mat_object(TV, TVMaterial);

    //Tv screen
    trans_matrix = translate(-2.45f, -1.48f, -3.39f);
    rot_matrix = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f));
    rot2_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
    scale_matrix = scale(0.666f, 0.666f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*rot2_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();


    if(channel == 1){
        draw_tex_object(Painting, Wednesday);
    }else if(channel == 2){
        draw_tex_object(Painting, Splatoon);
    }else if(channel == 3){
        draw_tex_object(Painting, Coyote);
    }




    //soda in cup
    trans_matrix = translate(0.4f, -3.07f, 0.2f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.175f, 0.13f, 0.175f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    draw_mat_object(Cup, SodaMaterial);



    //Fruit Bowl
    trans_matrix = translate(0.0f, -2.95f, -0.2f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.3f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    draw_mat_object(Bowl, WhiteMaterial);



    //Window
    trans_matrix = translate(1.0f, 0.0f, -3.90f);
    rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    rot2_matrix = rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));
    model_matrix = trans_matrix*rot_matrix*scale_matrix*rot2_matrix;

    draw_tex_object(Painting, Window);

    //Window frame
    trans_matrix = translate(0.9f, -1.0f, -3.90f);
    rot_matrix = rotate(0.0f, vec3(1.0f, 0.0f, 0.0f));
    scale_matrix = scale(1.4f, 1.2f, 1.5f);
    rot2_matrix = rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));
    model_matrix = trans_matrix*rot_matrix*scale_matrix*rot2_matrix;

    draw_tex_object(Frame, Wood);

    //Blinds
    trans_matrix = translate(1.0f, 1.2f, -3.45f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(1.0f, blinds_scale, 0.7f);
    rot2_matrix = rotate(180.0f, vec3(1.0f, 0.0f, 0.0f));
    model_matrix = trans_matrix*rot_matrix*rot2_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Blinds, WhiteMaterial);



    //Painting
    trans_matrix = translate(3.90f, -0.1f, 0.18f);
    rot_matrix = rotate(90.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //draw_color_obj(Cube, BlueCube);
    draw_tex_object(Painting, Popeye);

    //Painting frame
    trans_matrix = translate(3.85f, -1.2f, 0.1f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 1.18f, 1.4f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Frame, Wood);

    //Mirror
    if(!mirror) {
        trans_matrix = translate(mirror_eye);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(1.2f, 1.2f, 1.2f);
        rot2_matrix = rotate(0.0f, vec3(1.0f, 0.0f, 0.0f));
        model_matrix = trans_matrix * rot_matrix * scale_matrix * rot2_matrix;

        draw_tex_object(Plane, MirrorTex);
    }
    //Mirror Frame
    trans_matrix = translate(0.0f, -1.7f, 3.93f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.2f, 1.4f, 1.7f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Frame, Wood);


    //fan
    trans_matrix = translate(0.0f, 3.0f, 0.0f);
    rot_matrix = rotate(fan_angle, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.5f, 0.5f, 0.5f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Fan, WhiteMaterial);




    //Door

    trans_matrix = translate(-3.8f, -2.2f, 0.0f);
    rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(2.0f, 2.0f, 2.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Door, Wood, WoodNorm);

    //table
    trans_matrix = translate(0.0f, -4.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.75f, 0.75f, 0.75f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Table, Wood, WoodNorm);


    //chairs
    trans_matrix = translate(0.0f, -3.55f, 1.0f);
    rot_matrix = rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.25f, 0.35f, 0.25f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();

    draw_bump_object(Chair, Wood, WoodNorm);



    trans_matrix = translate(0.0f, -3.55f, -1.0f);
    rot_matrix = rotate(-90.0f, vec3(0.0f, 1.0f, 0.0f));
    scale_matrix = scale(0.25f, 0.35f, 0.25f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();

    draw_bump_object(Chair, Wood, WoodNorm);

//fruit
    trans_matrix = translate(-0.18f, -2.98f, -0.2f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;


    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Sphere, Apple, FruitNorm);


    trans_matrix = translate(0.16f, -2.97f, -0.2f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Sphere, Apple, FruitNorm);


    trans_matrix = translate(-0.0f, -2.95f, -0.01f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Sphere, Apple, FruitNorm);

    trans_matrix = translate(0.0f, -2.95f, -0.32f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_bump_object(Sphere, Apple, FruitNorm);

    //Cup
    trans_matrix = translate(0.4f, -3.01f, 0.2f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.2f, 0.2f, 0.2f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    glDepthMask(GL_FALSE);
    draw_mat_object(Cup, CupMaterial);
    glDepthMask(GL_TRUE);



}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_object(Cube);
    load_object(Cup);
    load_object(Soda);
    load_object(Circle);
    load_object(Bowl);
    load_object(Blinds);
    load_object(Fan);
    load_object(Frame);
    load_object(Drawer);
    load_object(TV);
    load_object(Plane);



    load_bump_object(Door);
    load_bump_object(Chair);
    load_bump_object(Table);
    load_bump_object(Sphere);

    build_painting();

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);


    // Build color buffers

    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 0.0f, 1.0f, 1.0f), BlueCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 1.0f, 0.0f, 1.0f), GreenCube);
}


void build_materials( ) {
    // Add materials to Materials vector

    MaterialProperties walls = {vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
                                       vec4(0.7f, 0.75f, 0.9f, 1.0f), //diffuse
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
                                       20.0f, //shininess
                                       {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties cup = {vec4(0.5f, 0.5f, 0.5f, 0.3f), //ambient
                                vec4(0.8f, 0.8f, 0.8f, 0.3f), //diffuse
                                vec4(1.0f, 1.0f, 1.0f, 0.3f), //specular
                                20.0f, //shininess
                                {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties whiteMaterial = {vec4(0.4f, 0.4f, 0.4f, 1.0f), //ambient
                              vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
                              vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
                              30.0f, //shininess
                              {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties soda = {
            vec4(0.1f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.45f, 0.3, 0.1f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties tv = {
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties dresserMaterial = {
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.75f, 0.5f, 0.4f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    Materials.push_back(walls);
    Materials.push_back(cup);
    Materials.push_back(whiteMaterial);
    Materials.push_back(soda);
    Materials.push_back(tv);
    Materials.push_back(dresserMaterial);


    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.3f, 0.35f, 0.55f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(0.4f, 0.3f, 0.0f, 1.0f), //specular
            vec4(0.0f, -1.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties spotLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //diffuse
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //specular
            vec4(0.0f, 7.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            3.0f,   //cutoff
            2.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };



    Lights.push_back(whitePointLight);
    Lights.push_back(spotLight);
    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}
void load_bump_object(GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;
    vector<vec3> tangents;
    vector<vec3> bitangents;

    // Load model and set number of vertices
    loadOBJ(objFiles[obj], vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // TODO: Compute tangents and bitangents
    computeTangentBasis(vertices, uvCoords, normals, tangents, bitangents);

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tangCoords*numVertices[obj], tangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*bitangCoords*numVertices[obj], bitangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_painting() {
    // Painting geometry
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };

    uvCoords = {
            {1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
    };

    // Set number of vertices
    numVertices[Painting] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[Painting]);
    glBindVertexArray(VAOs[Painting]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Painting][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[Painting], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Painting][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[Painting], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Painting][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[Painting], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth -= daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
        eye[1] = -2.5f;
        center[0] = eye[0] + cos(azimuth*DEG2RAD);
        center[1] = eye[1] + cos(elevation * DEG2RAD);
        center[2] = eye[2] + sin(azimuth*DEG2RAD);
    } else if (key == GLFW_KEY_D) {
        azimuth += daz;
        eye[1] = -2.5f;
        center[0] = eye[0] + cos(azimuth*DEG2RAD);
        center[1] = eye[1] + cos(elevation * DEG2RAD);
        center[2] = eye[2] + sin(azimuth*DEG2RAD);

    }

    // Adjust elevation angle
    if (key == GLFW_KEY_Z)
    {
        elevation -= del;
        if (elevation > 179.0)
        {
            elevation = 179.0;
        }
        center[1] = eye[1] + cos(elevation * DEG2RAD);
    }
    else if (key == GLFW_KEY_X)
    {
        elevation += del;
        if (elevation < 1.0)
        {
            elevation = 1.0;
        }
        center[1] = eye[1] + cos(elevation * DEG2RAD);
    }

    if (key == GLFW_KEY_W)
    {
        vec3 dir = center - eye;
        vec3 eyeTest = eye + (dir * 0.04f);
        if(eyeTest[0]  <= 2.90 && eyeTest[0]>= -2.90 && eyeTest[2] <= 2.90 && eyeTest[2]>= -2.90){
            eye = eye + (dir * 0.04f);
            eye[1] = -2.5f;
            center[0] = eye[0] + cos(azimuth*DEG2RAD);
            center[1] = eye[1] + cos(elevation * DEG2RAD);
            center[2] = eye[2] + sin(azimuth*DEG2RAD);
        }

    }
    else if (key == GLFW_KEY_S)
    {
        vec3 dir = center - eye;
        vec3 eyeTest = eye - (dir * 0.04f);
        if(eyeTest[0]  <= 2.90 && eyeTest[0]>= -2.90 && eyeTest[2] <= 2.90 && eyeTest[2]>= -2.90) {
            eye = eye - (dir * 0.04f);
            eye[1] = -2.5f;
            center[0] = eye[0] + cos(azimuth * DEG2RAD);
            center[1] = eye[1] + cos(elevation * DEG2RAD);
            center[2] = eye[2] + sin(azimuth * DEG2RAD);
        }
    }

    if(key == GLFW_KEY_L && action == GLFW_PRESS){
        lightOn[1] = !lightOn[1];
    }

    if(key == GLFW_KEY_F && action == GLFW_PRESS){
        fan = !fan;
    }

    if(key == GLFW_KEY_O && action == GLFW_PRESS){
        blinds = !blinds;
    }

    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        channel += 1;
        if(channel == 4){
            channel = 0;
        }
    }

    // Compute updated camera position
    GLfloat x, y, z;







}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}


unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad(GLuint shader, GLuint tex)
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[tex]);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

#include "utilfuncs.cpp"
