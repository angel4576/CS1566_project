/*
 * 
 *
 * An OpenGL source code template.
 */
 


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <math.h>

#include "../mylib/initShader.h"
#include "../mylib/linear_alg.h"


#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define PI 3.141592 

GLuint program;
//f:1 b:2 l:3 r:4 u:5 d:6 
//f':7 b':8 l':9 r':10 u':11 d':12

//matrices
GLuint ctm_location;
GLuint model_view_location;
GLuint projection_location;

//vectors
GLuint ambient_product_location;
GLuint diffuse_product_location;
GLuint specular_product_location;
GLuint light_position_location;

//floats
GLuint shine_location;
GLuint a_cons_location;
GLuint a_linear_location;
GLuint a_quad_location;

vec4 a_product = {0,0,0,1};//initialize
vec4 d_product = {0,0,0,1};
vec4 s_product = {0,0,0,1};

mat4 ctm;
mat4 light_ctm = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
mat4* ctm_array;
mat4 model_view = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
mat4 projection = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

//point source
vec4 light_position = {0, 4, 0, 1};//point

GLfloat shininess = 5;

//distance-attenuation
//1/(a+bd+cd^2)
GLfloat attenuation_constant = 1.0;
GLfloat attenuation_linear = 1.0;
GLfloat attenuation_quadratic = 0.0;


//eye at up
//vec4 eye = {1, 0.8, 1, 1};//vrp. x = 1, z = 1
vec4 eye = {2.5, 1.5, 2.5, 1};
vec4 at = {0, 0, 0, 1};//origin
vec4 up = {0, 1, 0, 0};//set(0, 1, 0, 0);//y-axis
//number for each face of the cube
int front[9] = {7, 4, 8, 1, 0, 2, 5, 3, 6};
int back[9] = {26, 22, 25, 20, 18, 19, 24, 21, 23};
int top[9] = {5, 3, 6, 14, 12, 15, 23, 21, 24};
int down[9] = {25, 22, 26, 16, 13, 17, 7, 4, 8};
int left[9] = {25, 16, 7, 19, 10, 1, 23, 14, 5};
int right[9] = {8, 17, 26, 2, 11, 20, 6, 15, 24};
//for animation
typedef enum{
  NONE = 0,
  FRONT,
  BACK,
  LEFT,
  RIGHT,
  TOP,
  DOWN,
  FRONT_P,
  BACK_P,
  LEFT_P,
  RIGHT_P,
  TOP_P,
  DOWN_P,

}state;

int is_animate = 1;
int start_shuffle = 0;
state current_state = NONE;
float current_angle = 0;
float change_angle = PI/64;

int formula[30] = {1, 6, 6, 3, 3, 2, 6, 2, 2, 2, 1, 1,
                         5, 5, 5, 1, 5, 1, 1, 5, 5, 1, 
                         1, 1, 3, 6, 1, 1, 1, 5};;//a pattern formula
int step = 0;
int max_step = 30;

int is_shadow = 0;

vec4 *vertices;//vertices array
vec4 *colors;//colors array
vec4 *normals;//normals array

int num_vertices;
int one_cube_num = 36+96;//132
int one_sphere_num = 36*6*16 + 36*3*2;

float size = 0.5/2;
float length = 0.55/2;
float edge_length;

float r = 1;//radius
//position when click
GLfloat init_x;
GLfloat init_y;
GLfloat init_z;
vec4 init_p;
vec4 origin = {0,0,0,1};//origin

//1/(a+bd+cd^2)
GLfloat attenuation(GLfloat d){//d = magnitude(light_position - vertex_pos);
  return 1/(attenuation_constant + (attenuation_linear * d) 
            + (attenuation_quadratic * d * d));
}


float calculate_vel(vec4 axis, vec4 lookat){
  //vec4 e_a = vec_vec_sub(eye, at);
  //vec4 axis = set(0,0,1,0);
  float dot = dot_product(lookat, axis);
  float co = dot / (vec_mag(lookat) * vec_mag(axis));
 
  return co;
}

mat4 identity(){
  mat4 i = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
  return i;
}

vec4 set(float a, float b, float c, float d){
  vec4 v = {a,b,c,d};
  return v;
}

void draw_shadow(){
  //calculate shadow vertice
  int s = 27*one_cube_num + 6;
  float sh_x;
  float sh_z;
  //xs= xl−(yl−ys)(xl-x)/(yl-y)
  for(int i = s; i < s+27*one_cube_num; i++){//shadow vertices
    sh_x = light_position.x - (light_position.y - (-1.5))
            * (light_position.x - vertices[i-s].x)/(light_position.y - vertices[i-s].y);
    
    sh_z = light_position.z - (light_position.y - (-1.5))
            * (light_position.z - vertices[i-s].z)/(light_position.y - vertices[i-s].y);
    
    vertices[i] = set(sh_x, -1.5, sh_z, 1);

    colors[i] = set(0,0,0,1);

  }
}

void draw_ground(){
  int g = 27*one_cube_num;
  vertices[g] = set(-4,-1.5, 4, 1);
  vertices[g+1] = set(4,-1.5, 4, 1);
  vertices[g+2] = set(4,-1.5, -4, 1);
  vertices[g+3] = vertices[g];
  vertices[g+4] = vertices[g+2];
  vertices[g+5] = set(-4,-1.5, -4, 1);
  
  //set color
  for(int i = g; i < g+6; i++){
    colors[i] = set(1,1,1,1);
  }
}

mat4 turn_around_y(float size, int option){
  mat4 ry = rotate_y(PI);
  mat4 t;
  if(option == 1){//the square is on x axis
    t = trans_m(size,0,0);
  }else{//square on z
    t = trans_m(0,0,-size);
  }
  return mat_mat_mul(t, ry);
}

int triangle_i(int num){
  return (num*3 + 36);
}


void set_light_parameters(){
  //light source
  vec4 light_ambient = {0.3,0.3,0.3,1};//L_ra//initialize
  vec4 light_diffuse = {0.3,0.3,0,1};
  vec4 light_specular = {0.3,0.3,0,1};

  //reflect
  vec4 reflect_ambient = {1,1,1,1};//k_ra
  vec4 reflect_diffuse = {0,0.1,0,1};
  vec4 reflect_specular = {0,0.1,0,1};

  a_product = product(reflect_ambient, light_ambient);
  //a_product = set(0.3, 0.3, 0.3, 1);
  d_product = product(reflect_diffuse, light_diffuse);
  s_product = product(reflect_specular, light_specular);

}

void build_edge(vec4 *v){
  //6*6
  /*-----------building edge triangles-----------*/
  //four triangles in the front
  for(int i = 36; i < 36+12; i+=3){
    vec4 temp = v[(i-36)/3];
    //v[i] = v[(i-36)/3];
    if(i == 36){//left bottom corner
      v[i] = mat_vec_mul(trans_m(-(length-size),0,-(length-size)) ,temp);//left
      v[i+1] = mat_vec_mul(trans_m(0,-(length-size),-(length-size)) ,temp);//down
      v[i+2] = temp;
    }else if(i == 39){//second, right bottom corner
      v[i] = temp;
      v[i+1] = mat_vec_mul(trans_m(0,-(length-size),-(length-size)) ,temp);
      v[i+2] = mat_vec_mul(trans_m((length-size),0,-(length-size)) ,temp);
    }else if(i == 42){//left top
      v[i] = mat_vec_mul(trans_m(0,(length-size),-(length-size)) ,temp);
      v[i+1] = mat_vec_mul(trans_m(-(length-size),0,-(length-size)) ,temp);
      v[i+2] = temp;
    }else if(i == 45){//right top
      temp = v[5];
      v[i] = mat_vec_mul(trans_m(0,(length-size),-(length-size)) ,temp);
      v[i+1] = temp;
      v[i+2] = mat_vec_mul(trans_m((length-size),0,-(length-size)) ,temp);
      
      
    }
  }
  //four in the back
  for(int i = 48; i < 48+12; i+=3){
    //2(LENGTH-SIZE)+size = 2length - size
    vec4 temp;
    if(i == 48){
      temp = mat_vec_mul(trans_m(0,0,-(2*length-size)), v[1]);
      //v[i] = mat_vec_mul(trans_m(0,0,-(2*length-size)), v[1]);
      v[i] = mat_vec_mul(trans_m((length-size),0,(length-size)) ,temp);
      v[i+1] = mat_vec_mul(trans_m(0,-(length-size),(length-size)) ,temp);
      v[i+2] = temp;
    }else if(i == 51){
      temp = mat_vec_mul(trans_m(0,0,-(2*length-size)), v[0]);
      v[i] = temp;
      v[i+1] = mat_vec_mul(trans_m(0,-(length-size),(length-size)) ,temp);
      v[i+2] = mat_vec_mul(trans_m(-(length-size),0,(length-size)) ,temp);
    }else if(i == 54){
      temp = mat_vec_mul(trans_m(0,0,-(2*length-size)), v[5]);
      v[i] = mat_vec_mul(trans_m(0,(length-size),(length-size)) ,temp);
      v[i+1] = mat_vec_mul(trans_m((length-size),0,(length-size)) ,temp);
      v[i+2] = temp;
      
    }else if(i == 57){
      temp = mat_vec_mul(trans_m(0,0,-(2*length-size)), v[2]);
      v[i] = mat_vec_mul(trans_m(0,(length-size),(length-size)) ,temp);
      v[i+1] = temp;
      v[i+2] = mat_vec_mul(trans_m(-(length-size),0,(length-size)) ,temp);
      
    }
  }
  /*-----------building edge rectangles-----------*/
  for(int i = 60; i < 60+24; i+=6){
    int index = (i-60)/6;
    int next_i;
    if(index == 0){
      next_i = 1;
    }else if(index == 1){
      next_i = 4;
    }else if(index == 2){
      index = 4;
      next_i = 5;
    }else if(index == 3){
      index = 5;
      next_i = 0;
    }
    //first
    v[i] = v[triangle_i(index)+2];
    v[i+1] = v[triangle_i(index)+1];
    v[i+2] = v[triangle_i(next_i)];//

    v[i+3] = v[i+3-1];//fourth. previous point
    v[i+4] = v[i+4-3];
    v[i+5] = v[triangle_i(next_i)+1];
  }
  //top edges
  //triangle order: 2 3 6 7
  for(int i = 84; i < 84+24; i+=6){
    int index;
    int next_i;
    if(i == 84){
      index = 2;
      next_i = 3;
    }else if(i == 90){
      index = 3;
      next_i = 6;
    }else if(i == 96){
      index = 6;
      next_i = 7;
    }else if(i == 102){
      index = 7;
      next_i = 2;
    }
    
    v[i] = v[triangle_i(index)];
    v[i+1] = v[triangle_i(index)+2];
    v[i+2] = v[triangle_i(next_i)];//

    v[i+3] = v[i+3-1];//fourth. previous point
    v[i+4] = v[i+4-3];
    v[i+5] = v[triangle_i(next_i)+1];
  }
 //vertical edges
 for(int i = 108; i < 108+24; i+=6){
   int index;
    int next_i;
    if(i == 108){
      index = 2;
      next_i = 0;
    }else if(i == 108+6){
      index = 3;
      next_i = 1;
    }else if(i ==108+12){
      index = 6;
      next_i = 4;
    }else if(i == 108+18){
      index = 7;
      next_i = 5;
    }

    v[i] = v[triangle_i(index)+2];
    v[i+1] = v[triangle_i(index)+1];
    v[i+2] = v[triangle_i(next_i)+2];//

    v[i+3] = v[i+3-1];//fourth. previous point
    v[i+4] = v[i+4-3];
    v[i+5] = v[triangle_i(next_i)];
 }

}

void build_two_layers(){
  mat4 t = trans_m(0,0,-edge_length);
  for(int i = 9*one_cube_num; i < 18*one_cube_num; i++){
    vertices[i] = mat_vec_mul(t, vertices[i-9*one_cube_num]);
  }

  mat4 t_2 = trans_m(0,0,-2*edge_length);
  for(int i = 18*one_cube_num; i < 27*one_cube_num; i++){
    vertices[i] = mat_vec_mul(t_2, vertices[i-18*one_cube_num]);
  }

}

void build_first_layer(){
  //printf("edge length: %f\n", edge_length);
  //second cube. left to the center
  for(int i = one_cube_num; i < 2*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(-edge_length,0,0), vertices[i-one_cube_num]);
  }
  //third. right
  for(int i = 2*one_cube_num; i < 3*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(edge_length,0,0), vertices[i-2*one_cube_num]);
  }
  //forth. up
  for(int i = 3*one_cube_num; i < 4*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(0,edge_length,0), vertices[i-3*one_cube_num]);
  }
  //fifth. down
  for(int i = 4*one_cube_num; i < 5*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(0,-edge_length,0), vertices[i-4*one_cube_num]);
  }
  //sixth. top left
  for(int i = 5*one_cube_num; i < 6*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(-edge_length,edge_length,0), vertices[i-5*one_cube_num]);
  }
  //seventh. top right
  for(int i = 6*one_cube_num; i < 7*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(edge_length,edge_length,0), vertices[i-6*one_cube_num]);
  }
  //eighth. down left
  for(int i = 7*one_cube_num; i < 8*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(-edge_length,-edge_length,0), vertices[i-7*one_cube_num]);
  }
  //ninth. down right
  for(int i = 8*one_cube_num; i < 9*one_cube_num; i++){
    vertices[i] = mat_vec_mul(trans_m(edge_length,-edge_length,0), vertices[i-8*one_cube_num]);
  }
}

void set_cube_vertex(vec4 *v){
    mat4 r_y = rotate_y(PI/2);
    mat4 r_z = rotate_z(PI/2);
    mat4 r_x = rotate_x(PI/2);

    mat4 t_z = trans_m(0,0,-length);
    mat4 t_x = trans_m(length,0,0);

    vec4 *temp = (vec4*)malloc(sizeof(vec4)*6);

    temp[0] = set(0,0,0,1);
    temp[1] = set(size,0,0,1);
    temp[2] = set(0,size,0,1);
    temp[3] = set(0,size,0,1);
    temp[4] = set(size,0,0,1);
    temp[5] = set(size,size,0,1);

    for(int i = 0; i < 36; i++){
      if(i >= 0 && i < 6){
        v[i] = mat_vec_mul(trans_m(0,0,(length-size)), temp[i]);
      }else if(i >= 6 && i < 12){//back
        mat4 turn_t = mat_mat_mul(t_z, turn_around_y(size, 1));
        v[i] = mat_vec_mul(turn_t, temp[i-6]);

      }else if(i >= 2*6 && i < 3*6){//third face, right
        v[i] = mat_vec_mul(r_y, temp[i-2*6]);
        v[i] = mat_vec_mul(t_x, v[i]);

      }else if(i >= 3*6 && i < 4*6){//four face, left
        mat4 turn = mat_mat_mul(trans_m(-(length-size),0,-size) ,rotate_y(-PI/2));//turn_around_y(length, 2);
        v[i] = mat_vec_mul(turn, temp[i-3*6]);
        
      }else if(i >= 4*6 && i < 5*6){//fifth, bottom
        mat4 ts = mat_mat_mul(trans_m(0,-(length-size),-size), rotate_x(PI/2));
        v[i] = mat_vec_mul(ts, temp[i-4*6]);//rotate from the front
      
      }else if(i >= 5*6 && i < 6*6){//top
        mat4 ts = mat_mat_mul(trans_m(0,length,0) ,rotate_x(-PI/2));
        v[i] = mat_vec_mul(ts, temp[i-5*6]);
      }
    }
    
    build_edge(vertices);
    
}

void set_cube_color(vec4 *c, int num){
  /*
  vec4 ambient;
  vec4 diffuse = {0,0,0,1};
  vec4 specular = {0,0,0,1};
  //calculate ambient
  ambient = product(light_ambient, reflect_ambient);
  //calculate normal
  vec4 n = set(0,0,0,0);//need to change. based on vertex
  vec4 l = vec_vec_sub(light_position, set(0,0,0,1)); //distance of distance light source
  //diffuse
  GLfloat d = dot_product(n, normalize(l));
  if(d > 0.0)//max(l . n, 0)
    diffuse = scalar_vec_mul(d, product(light_diffuse, reflect_diffuse));

  //specular
  vec4 v = normalize(vec_vec_sub(eye, set(0,0,0,1)));//a vertex to eye point
  vec4 half = normalize(vec_vec_add(l, v));//half vector. l+v
  GLfloat s = dot_product(half, n);
  if(s > 0.0)
    specular = scalar_vec_mul(pow(s, shininess), product(light_specular, reflect_specular));  
  */

  for(int i = num; i < num+36; i++){
      if(i >= num && i < num+6){//front
        c[i] = set(0,1,0,1);
      }else if(i >= num+6 && i < num+12){//back
        c[i] = set(0,0,1,1);
      }else if(i >= num+12 && i < num+18){//third face, right
        c[i] = set(1,0,0,1);
      }else if(i >= num+18 && i < num+24){//four face, left
        c[i] = set(1,0.5,0,1);
      }else if(i >= num+24 && i < num+30){//fifth, bottom
        c[i] = set(1,1,0.3,1);
      }else if(i >= num+30 && i < num+36){//top
        c[i] = set(1,1,1,1);
      }
  }

   for(int i = num+36; i < num+one_cube_num; i++){
      c[i] = set(0,0,0,1);
    }
}

void set_cube_normal(vec4 *n){
  for(int i = 0; i < 27*one_cube_num+6; i+=3){
    //triangle
    vec4 p0 = vertices[i];
    vec4 p1 = vertices[i+1];
    vec4 p2 = vertices[i+2];
    
    //calculate normal
    vec4 u = vec_vec_sub(p1, p0);
    vec4 v = vec_vec_sub(p2, p1);

    vec4 normal = cross_product(u, v);
    normal = normalize(normal);

    n[i] = normal;
    n[i+1] = normal;
    n[i+2] = normal;

  }

  for(int i = 27*one_cube_num+6; i < num_vertices; i++){
    n[i] = set(0,0,0,0);
  }

}

void translate_cube(){
  edge_length = size + 2*(length-size);
  //translate to the origin
  mat4 t = trans_m(-size/2, -size/2, 2*(length-size)+3*size/2);
  for(int i = 0; i < one_cube_num; i++){
    vertices[i] = mat_vec_mul(t, vertices[i]);
  }
}

void set_sphere_vertex(vec4 *v, int start){    
    int j = 0;
    int i = start;//0;
    int count = 36*6;
    
    v[start] = set(1,0,0,1);//on the x axis
    vec4 temp = v[start];
    
    
    //upper half of the sphere
    while(j < 8){
    
      vec4 up = mat_vec_mul(rotate_z(PI/18), temp);//3rd
      for(; i < start+count; i+=6){//one band
        v[i] = temp;
        v[i+1] = mat_vec_mul(rotate_y(PI/18), temp);//2nd
        v[i+2] = up;//3rd
      
        v[i+3] = v[i+2];//equals to the 3rd
        v[i+4] = v[i+1];//equals to the 2nd
        v[i+5] = mat_vec_mul(rotate_y(PI/18), v[i+2]);//rotate from the 3rd
    
        temp = mat_vec_mul(rotate_y(PI/18), temp);
        up = mat_vec_mul(rotate_y((PI/18)), up);
        
      
      }
      temp = mat_vec_mul(rotate_z(PI/18), temp);
      j++;
      
      count += 216;
    }
    
    vec4 top_st = v[i-1];
    for(; i < start+one_sphere_num/2; i+=3){
      v[i] = top_st;
      v[i+1] = mat_vec_mul(rotate_y((PI/18)), v[i]);
      v[i+2] = set(0,1,0,1);//tip
      
      top_st = v[i+1];
    }
    
    count+= 36*3;
    
    //lower part 
    vec4 temp_2 = set(1,0,0,1);
    
    int j_2 = 0;
    while(j_2 < 8){
    
      vec4 down = mat_vec_mul(rotate_z(-PI/18), temp_2);//3rd
      for(; i < start+count; i+=6){
        v[i] = temp_2;
        v[i+1] = mat_vec_mul(rotate_y(-PI/18), temp_2);//2nd
        v[i+2] = down;//3rd
      
        v[i+3] = v[i+2];//equals to the 3rd
        v[i+4] = v[i+1];//equals to the 2nd
        v[i+5] = mat_vec_mul(rotate_y(-PI/18), v[i+2]);//rotate from the 3rd
    
        temp_2 = mat_vec_mul(rotate_y(-PI/18), temp_2);
        down = mat_vec_mul(rotate_y(-PI/18), down);
        
      
      }
      temp_2 = mat_vec_mul(rotate_z(-PI/18), temp_2);
      j_2++;
      
      count += 216;
      //printf("test\n");
    }
    
    vec4 bottom_st = v[i-1];
    for(; i < start+one_sphere_num; i+= 3){
      v[i] = bottom_st;
      v[i+1] = mat_vec_mul(rotate_y((-PI/18)), v[i]);
      v[i+2] = set(0,-1,0,1);//tip
      
      bottom_st = v[i+1];
    }
    
}

void set_sphere_color(vec4 *c, int start){
    //float ran_x;
    //float ran_y;// = (float)rand()/ (float)RAND_MAX;
    //float ran_z; //= (float)rand()/ (float)RAND_MAX;
    
    for(int i = start; i < start + one_sphere_num; i++){
      c[i] = set(1,1,1,1);
    }

    //random color
    //printf("%f", ran);
    /*
    for(int i = start; i < start + one_sphere_num; i+=3){
      ran_x = (float)rand()/ (float)RAND_MAX;
      ran_y = (float)rand()/ (float)RAND_MAX;
      ran_z = (float)rand()/ (float)RAND_MAX;
    
    
      c[i] = set(ran_x,ran_y,ran_z,1);
      c[i+1] = set(ran_x,ran_y,ran_z,1);
      c[i+2] = set(ran_x,ran_y,ran_z,1);
    }
    */
}

void translate_sphere(int start){
  mat4 trans = trans_m(light_position.x, light_position.y, light_position.z);
  trans = mat_mat_mul(trans, sca_m(0.2, 0.2, 0.2));
  for(int i = start; i < start+one_sphere_num; i++){
    vertices[i] = mat_vec_mul(trans, vertices[i]);
  }
}

GLfloat change_x(int x){
  GLfloat result;
  int offset;
  if(x == 256){
    return 0;
  }else if(x > 256){
    offset = x - 256;
    result = offset * (1.0f/256.0f);
    return result;
  }else if(x < 256){
    offset = x -256;
    result = offset * (1.0f/256.0f);
    return result;
  }

  return 0;

}

GLfloat change_y(int y){
  GLfloat result;
  int offset;
  if(y == 256){
    return 0;
  }else if(y > 256){
    offset = y - 256;
    result = offset * -(1.0f/256.0f);
    return result;
  }else if(y < 256){
    offset = y -256;
    result = offset * -(1.0f/256.0f);
    return result;
  }

  return 0;

}

void set_random_formula(){
  for(int i = 0; i < 30; i++){
    formula[i] = rand()%13;
  }
}

void turn_cube(int face){
  is_animate = 1;
  if(face == 1 || face == 7){//front
    if(face == 1){
      current_state = FRONT;
      int temp[9] = {right[0], right[3], right[6], down[7], 0, top[1], left[2], left[5], left[8]};
      for(int i = 0; i < 9; i++){
        front[i] = temp[i];
      }

    }else{//counter clockwise
      current_state = FRONT_P;
      int temp[9] = {left[8], left[5], left[2], top[1], 0, down[7], right[6], right[3], right[0]};
      for(int i = 0; i < 9; i++){
        front[i] = temp[i];
      }
    }
      //modify top
      top[0] = front[6];
      top[1] = front[7];
      top[2] = front[8];
      //modify right
      right[0] = front[2];
      right[3] = front[5];
      right[6] = front[8];
      //modify left
      left[2] = front[0];
      left[5] = front[3];
      left[8] = front[6];
      //modify down
      down[6] = front[0];
      down[7] = front[1];
      down[8] = front[2];

  }else if(face == 2 || face == 8){//back

    if(face == 2){
      current_state = BACK;
      int temp[9] = {left[0], left[3], left[6], down[1], 18, top[7], right[2], right[5], right[8]};
      for(int i = 0; i < 9; i++){
        back[i] = temp[i];
      }
    }else{
      current_state = BACK_P;
      int temp[9] = {right[8], right[5], right[2], top[7], 18, down[1], left[6], left[3], left[0]};
      for(int i = 0; i < 9; i++){
        back[i] = temp[i];
      }
    }

      //modify top
      top[6] = back[8];
      top[7] = back[7];
      top[8] = back[6];
      //modify right
      right[2] = back[0];
      right[5] = back[3];
      right[8] = back[6];
      //modify left
      left[0] = back[2];
      left[3] = back[5];
      left[6] = back[8];
      //modify down
      down[0] = back[2];
      down[1] = back[1];
      down[2] = back[0];

  }else if(face == 3 || face == 9){//left

    if(face == 3){
      current_state = LEFT;
      int temp[9] = {front[0], front[3], front[6], down[3], 10, top[3], back[2], back[5], back[8]};
      for(int i = 0; i < 9; i++){
        left[i] = temp[i];
      }
    }else{
      current_state = LEFT_P;
      int temp[9] = {back[8], back[5], back[2], top[3], 10, down[3], front[6], front[3], front[0]};
      for(int i = 0; i < 9; i++){
        left[i] = temp[i];
      }
    } 

      //modify front
      front[0] = left[2];
      front[3] = left[5];
      front[6] = left[8];
      //modify top
      top[6] = left[6];
      top[3] = left[7];
      top[0] = left[8];
      //modify down
      down[0] = left[0];
      down[3] = left[1];
      down[6] = left[2];
      //modify back
      back[2] = left[0];
      back[5] = left[3];
      back[8] = left[6];
      
  }else if(face == 4 || face == 10){//right

    if(face == 4){
      current_state = RIGHT;
      int temp[9] = {back[0], back[3], back[6], down[5], 11, top[5], front[2], front[5], front[8]};
      for(int i = 0; i < 9; i++){
        right[i] = temp[i];
      }

    }else{
      current_state = RIGHT_P;
      int temp[9] = {front[8], front[5], front[2], top[5], 11, down[5], back[6], back[3], back[0]};
      for(int i = 0; i < 9; i++){
        right[i] = temp[i];
      }
    }
      //modify front
      front[2] = right[0];
      front[5] = right[3];
      front[8] = right[6];
      //modify top
      top[2] = right[6];
      top[5] = right[7];
      top[8] = right[8];
      //modify down
      down[2] = right[2];
      down[5] = right[1];
      down[8] = right[0];
      //modify back
      back[0] = right[2];
      back[3] = right[5];
      back[6] = right[8];
      
  }else if(face == 5 || face == 11){//top
    if(face == 5){
      current_state = TOP;
      int temp[9] = {right[6], right[7], right[8], front[7], 12, back[7], left[8], left[7], left[6]};
      for(int i = 0; i < 9; i++){
        top[i] = temp[i];
      }
    }else{
      current_state = TOP_P;
      int temp[9] = {left[6], left[7], left[8], back[7], 12, front[7], right[8], right[7], right[6]};
      for(int i = 0; i < 9; i++){
        top[i] = temp[i];
      }
    }
      //modify front
      front[6] = top[0];
      front[7] = top[1];
      front[8] = top[2];
      //modify right
      right[6] = top[2];
      right[7] = top[5];
      right[8] = top[8];
      //modify left
      left[8] = top[0];
      left[7] = top[3];
      left[6] = top[6];
      //modify back
      back[6] = top[8];
      back[7] = top[7];
      back[8] = top[6];

  }else if(face == 6 || face == 12){//down
    if(face == 6){
      current_state = DOWN;
      int temp[9] = {right[2], right[1], right[0], back[1], 13, front[1], left[0], left[1], left[2]};
      for(int i = 0; i < 9; i++){
        down[i] = temp[i];
      }
    }else{
      current_state = DOWN_P;
      int temp[9] = {left[2], left[1], left[0], front[1], 13, back[1], right[0], right[1], right[2]};
      for(int i = 0; i < 9; i++){
        down[i] = temp[i];
      }
    }
      //modify front
      front[0] = down[6];
      front[1] = down[7];
      front[2] = down[8];
      //modify right
      right[0] = down[8];
      right[1] = down[5];
      right[2] = down[2];
      //modify left
      left[0] = down[0];
      left[1] = down[3];
      left[2] = down[6];
      //modify back
      back[0] = down[2];
      back[1] = down[1];
      back[2] = down[0];
  
  }
} 


void init(void)
{   
    //cube
    num_vertices = 27*one_cube_num;//magic cube ver_num
    num_vertices += 6;//ground
    num_vertices += one_sphere_num;//num of vertices of sphere

    vertices = (vec4*)malloc(sizeof(vec4)*num_vertices);
    colors = (vec4*)malloc(sizeof(vec4)*num_vertices);
    normals = (vec4*)malloc(sizeof(vec4)*num_vertices);
    
    ctm_array = (mat4*)malloc(sizeof(mat4)*27);
    for(int i = 0; i < 27; i++){
      ctm_array[i] = identity();
    }

    //set model view
    model_view = look_at(eye, at, up);
    //set frustrum
    projection = frustum(-0.25, 0.25, -0.30, 0.2, -0.5, -20);//left, right, bottom, top, near, far
    
    //set radius of viewing sphere
    vec4 lookat = vec_vec_sub(eye, at);
    r = vec_mag(lookat);

    //generate random shuffle formula
    set_random_formula();

    //build magic cube
    set_cube_vertex(vertices);//set one cube vertex
    translate_cube();
    build_first_layer();
    build_two_layers();
    //set all cubes colors
    for(int i = 0; i < 27; i++){
      set_cube_color(colors, i*one_cube_num);//pass the start vertex of each cube
    }

    draw_ground();
    
    set_sphere_vertex(vertices, 27*one_cube_num + 6);
    set_sphere_color(colors, 27*one_cube_num + 6);
    translate_sphere(27*one_cube_num + 6);//translate to light position


    //set normal array
    set_cube_normal(normals);
    
    /*
    //print normals
    for (int i = 0; i < num_vertices; i++)
    {
      printf("%d ", i);
      print_vec(normals[i]);
    }
    */
    
    //print color for debug
    /*
    for(int i = 0; i < num_vertices; i++){
      printf("%d ", i);
      print_vec(colors[i]);
    }
    */

    //printf("edge length: %f\n", edge_length);
    //print vertices array
    /*
    for(int i = 0; i < num_vertices; i++){
      printf("%d ", i);
      print_vec(vertices[i]);
    }
    */
  
  
    program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 3*sizeof(vec4)*num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4)*num_vertices, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*num_vertices, sizeof(vec4)*num_vertices, colors);
    glBufferSubData(GL_ARRAY_BUFFER, 2*sizeof(vec4)*num_vertices, sizeof(vec4)*num_vertices, normals);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4)*num_vertices));

    //send normals array
    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (2*sizeof(vec4)*num_vertices));

    //send matrix
    ctm_location = glGetUniformLocation(program, "ctm");//ctm
    ctm = identity();//identity matrix
    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");

    //send vectors
    ambient_product_location = glGetUniformLocation(program, "ambient_product");
    diffuse_product_location = glGetUniformLocation(program, "diffuse_product");
    specular_product_location = glGetUniformLocation(program, "specular_product");
    light_position_location = glGetUniformLocation(program, "light_position");

    //send floats
    shine_location = glGetUniformLocation(program, "shininess");
    a_cons_location = glGetUniformLocation(program, "attenuation_constant");
    a_linear_location = glGetUniformLocation(program, "attenuation_linear");
    a_quad_location = glGetUniformLocation(program, "attenuation_quadratic");

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 0.8, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);
    //ctm
    //glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm);//send  matrices  into  the  graphic  pipeline 
    
    //model view
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    //projection
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);
    
    //light parameters
    glUniform4fv(ambient_product_location, 1, (GLfloat *) &a_product);
    glUniform4fv(diffuse_product_location, 1, (GLfloat *) &d_product);
    glUniform4fv(specular_product_location, 1, (GLfloat *) &s_product);
    glUniform4fv(light_position_location, 1, (GLfloat *) &light_position);

    //shiniess and attenuation
    glUniform1fv(shine_location, 1, (GLfloat *) &shininess);
    glUniform1fv(a_cons_location, 1, (GLfloat *) &attenuation_constant);
    glUniform1fv(a_linear_location, 1, (GLfloat *) &attenuation_linear);
    glUniform1fv(a_quad_location, 1, (GLfloat *) &attenuation_quadratic);


    //CTMs
    glUniform1i(glGetUniformLocation(program, "is_shadow"), 0);
    for(int i = 0; i < 27; i++){
      //cube
      glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm_array[i]);//send  matrices  into  the  graphic  pipeline 
      
      glUniform1i(glGetUniformLocation(program, "is_shadow"), 0);
      glDrawArrays(GL_TRIANGLES, i*one_cube_num, one_cube_num);
      //draw shadow
      glUniform1i(glGetUniformLocation(program, "is_shadow"), 1);
      glDrawArrays(GL_TRIANGLES, i*one_cube_num, one_cube_num);

    }
    //draw ground
    glUniform1i(glGetUniformLocation(program, "is_shadow"), 0);
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm);
    glDrawArrays(GL_TRIANGLES, 27*one_cube_num, 6);

    //draw sphere
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &light_ctm);
    glDrawArrays(GL_TRIANGLES, 27*one_cube_num + 6, one_sphere_num);
    
    //shadow
    /*
    glUniform1i(glGetUniformLocation(program, "is_shadow"), 1);
    for(int i = 0; i < 27; i++){
      glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm_array[i]);//send  matrices  into  the  graphic  pipeline 
      glDrawArrays(GL_TRIANGLES, i*one_cube_num, one_cube_num);
    }
    */
    glutSwapBuffers();
}

float calculate_z(float x, float y){
  return (float)sqrt(1 - pow(x,2) - pow(y, 2));
}

void mouse(int button, int state, int x, int y){
  if(button == 3){//zoom in
    printf("zoom in\n");

    vec4 lookat = vec_vec_sub(eye, at);
    r = vec_mag(lookat);
    printf("%f\n", r);
    lookat = normalize(lookat);
   
    eye.x -= lookat.x/50;
    eye.y -= lookat.y/50;
    eye.z -= lookat.z/50;

    model_view = look_at(eye, at, up);
  }

  if(button == 4){
    printf("zoom out\n");

    vec4 lookat = vec_vec_sub(eye, at);
    r = vec_mag(lookat);
    printf("%f\n", r);
    lookat = normalize(lookat);
    //printf("%f\n", lookat.x);
    //printf("%f\n", lookat.y);
    //printf("%f\n", lookat.z);

    eye.x += lookat.x/50;
    eye.y += lookat.y/50;
    eye.z += lookat.z/50;

    model_view = look_at(eye, at, up);
  }

  glutPostRedisplay(); 

}

void rotate_about(vec4 about){
   about = normalize(about);
   mat4 M;
   if(about.y == 0 && about.z == 0){
     M = rotate_x(PI/512);
   }else{
    //printf("about: \n");
    //print_vec(about);
     float d = (float)sqrt(pow(about.y,2) + pow(about.z,2));
     //printf("d: %f\n", d);
     mat4 r_x = {{1,0,0,0},{0, about.z/d, about.y/d, 0},{0, -about.y/d, about.z/d,0},{0,0,0,1}};//rotate v to the plane y = 0
     
     vec4 temp = mat_vec_mul(r_x, about);//rotate about x
     //printf("temp\n");
     //print_vec(temp);
   
   
     mat4 r_y = {{d,0,temp.x,0},{0,1,0,0},{-temp.x,0,d,0},{0,0,0,1}};//rotate v to the plane x = 0; Ry(-theta_y)
    
     //printf("temp2\n");
     //print_vec(temp);
   
     mat4 r_z = rotate_z(PI/512);
   
     
     mat4 r_y_back = {{d,0,-temp.x,0},{0,1,0,0},{temp.x,0,d,0},{0,0,0,1}};//Ry(theta_y)
     temp = mat_vec_mul(r_y_back, temp);
     //printf("temp3\n");
     //print_vec(temp);
   
     mat4 r_x_back = {{1,0,0,0},{0, about.z/d, -about.y/d, 0},{0, about.y/d, about.z/d,0},{0,0,0,1}};//Rx(-theta_x)
     //temp = mat_vec_mul(r_x_back, temp);
     //printf("temp4\n");
     //print_vec(temp);
    

     mat4 M = r_x;
     M = mat_mat_mul(r_y, M);
     M = mat_mat_mul(r_z, M);
     M = mat_mat_mul(r_y_back, M);
     M = mat_mat_mul(r_x_back, M);
   }
   ctm = mat_mat_mul(M, ctm);   
  
   
}

void motion(int x, int y){
  //printf("initial: %f %f %f\n", init_x, init_y, init_z);
  GLfloat move_x = change_x(x);
  GLfloat move_y = change_y(y);
  GLfloat move_z = calculate_z(move_x, move_y);
  //printf("move: %f %f %f\n", move_x, move_y, move_z);
  
  
  if(isnan(move_z))return;
  //if(is_valid == 1){
  init_p = set(init_x, init_y, init_z, 1);//click point
  vec4 ori_v = vec_vec_sub(origin, init_p);//vector from click to origin
  //printf("origin: \n");
  //print_vec(ori_v);
  
  vec4 move_p = set(move_x, move_y, move_z, 1);//move point
  vec4 move_to = vec_vec_sub(move_p, init_p);//vector pointing from click point to next point
  //printf("move to: \n");
  //print_vec(move_to);
  
 
  vec4 about_v = normalize(cross_product(move_to, ori_v));//cross product -> get about vector
  //printf("about: \n");
  //print_vec(about_v);
  
  //rotate 
  rotate_about(about_v);
  //}
  glutPostRedisplay(); 
  

}

void specialkey(int key, int x, int y){
  if(key == GLUT_KEY_UP){
    printf("up\n");
  
    if(eye.y <= r){//limit the extent of looking up
      printf("%f\n", eye.y);
      eye.y+=0.09;
      model_view = look_at(eye, at, up);
    }
  }

  if(key == GLUT_KEY_DOWN){
      printf("down\n");
    if(eye.y >= -r){
      printf("%f\n", eye.y);
      eye.y-=0.09;
      model_view = look_at(eye, at, up);
    }
  }

  if(key == GLUT_KEY_LEFT){
      printf("go left\n");
      mat4 ro = rotate_y(-PI/90);
      eye = mat_vec_mul(ro, eye);
      model_view = look_at(eye, at, up);
  }

  if(key == GLUT_KEY_RIGHT){
     printf("go right\n");
      mat4 ro = rotate_y(PI/90);
      eye = mat_vec_mul(ro, eye);
      model_view = look_at(eye, at, up);
  }

}


void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q'){
    	glutLeaveMainLoop();
    }

    if(key == 's'){//shuffle
      start_shuffle = 1;
      is_animate = 0;
    }

    if(key == 'f'){//front
      turn_cube(1);
    }

    if(key == 'F'){//front. counter clockwise
      turn_cube(7);
    }

    if(key == 'b'){//back
      turn_cube(2);
    }

    if(key == 'B'){//back. counter clockwise
      turn_cube(8);
    }

    if(key == 'u'){//up
      turn_cube(5);
    }

    if(key == 'U'){//up
      turn_cube(11);
    }

    if(key == 'd'){//down
      turn_cube(6);
    }

    if(key == 'D'){//down
      turn_cube(12);
    }

    if(key == 'l'){
      turn_cube(3);
    }

    if(key == 'L'){
      turn_cube(9);
    }

    if(key == 'r'){
      turn_cube(4);
    }

    if(key == 'R'){
      turn_cube(10);
    }

    if(key == 'x'){
      light_position.x+=0.1;
      light_ctm = mat_mat_mul(trans_m(0.1, 0, 0), light_ctm);
    }

    if(key == 'X'){
      light_position.x-=0.1;
      light_ctm = mat_mat_mul(trans_m(-0.1, 0, 0), light_ctm);
    }

    if(key == 'y'){
      light_position.y+=0.1;
      light_ctm = mat_mat_mul(trans_m(0, 0.1, 0), light_ctm);
    }

    if(key == 'Y'){
      light_position.y-=0.1;
      light_ctm = mat_mat_mul(trans_m(0, -0.1, 0), light_ctm);
    }

    if(key == 'z'){
      light_position.z+=0.1;
      light_ctm = mat_mat_mul(trans_m(0, 0, 0.1), light_ctm);
    }

     if(key == 'Z'){
      light_position.z-=0.1;
      light_ctm = mat_mat_mul(trans_m(0, 0, -0.1), light_ctm);
    }

    glutPostRedisplay();
}



void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
}

void idle(void){
  //float times = (PI/2)/(change_angle);
  //int times = 64; //PI/2 / PI/128;
  //printf("%f\n", times);
  if(is_animate){

    if(current_state == LEFT || current_state == LEFT_P){
      current_angle += change_angle;
      //SET CTMs
      for(int i = 0; i < 9; i++){
        if(current_state == LEFT){
          ctm_array[left[i]] = mat_mat_mul(rotate_x(change_angle), ctm_array[left[i]]);//set ctms
        }else{
          ctm_array[left[i]] = mat_mat_mul(rotate_x(-change_angle), ctm_array[left[i]]);//set ctms 
        }
      }

      if(current_angle >= PI/2){
        current_angle = 0;
        //current_state = NONE;
        is_animate = 0;
      }

    }else if(current_state == RIGHT || current_state == RIGHT_P){
      current_angle += change_angle;
      for(int i = 0; i < 9; i++){
        if(current_state == RIGHT){
          ctm_array[right[i]] = mat_mat_mul(rotate_x(-change_angle), ctm_array[right[i]]);//set ctms
        }else{
          ctm_array[right[i]] = mat_mat_mul(rotate_x(change_angle), ctm_array[right[i]]);//set ctms
        }
      }

      printf("stop: %f\n", current_angle);
      if(current_angle >= PI/2){
        printf("stop: %f\n", current_angle);
        current_angle = 0;
        //current_state = NONE;
        is_animate = 0;
      }
    
    }else if(current_state == FRONT || current_state == FRONT_P){
      current_angle += change_angle;
      printf("angle: %f\n", current_angle);
      //SET CTMs
      for(int i = 0; i < 9; i++){
        if(current_state == FRONT){
          ctm_array[front[i]] = mat_mat_mul(rotate_z(-change_angle), ctm_array[front[i]]);
        }else{
          ctm_array[front[i]] = mat_mat_mul(rotate_z(change_angle), ctm_array[front[i]]);
        }
      }

      if(current_angle >= PI/2){
      //if(float_equal(current_angle, PI/2)){
        printf("angle: %f\n", current_angle);
        current_angle = 0;
        //current_state = NONE;
        is_animate = 0;
      }

    }else if(current_state == BACK || current_state == BACK_P){
      current_angle += change_angle;
      //SET CTMs
      for(int i = 0; i < 9; i++){
        if(current_state == BACK){
          ctm_array[back[i]] = mat_mat_mul(rotate_z(change_angle), ctm_array[back[i]]);
        }else{
          ctm_array[back[i]] = mat_mat_mul(rotate_z(-change_angle), ctm_array[back[i]]);
        }
      }

      if(current_angle >= PI/2){
        current_angle = 0;
        is_animate = 0;
        //current_state = NONE;
      }

    }else if(current_state == TOP || current_state == TOP_P){
      current_angle += change_angle;
      //SET CTMs
      for(int i = 0; i < 9; i++){
        if(current_state == TOP){
          ctm_array[top[i]] = mat_mat_mul(rotate_y(-change_angle), ctm_array[top[i]]);
        }else{
          ctm_array[top[i]] = mat_mat_mul(rotate_y(change_angle), ctm_array[top[i]]);

        }
      }

      if(current_angle >= PI/2){
        current_angle = 0;
        is_animate = 0;
        //current_state = NONE;
      }

    }else if(current_state == DOWN || current_state == DOWN_P){
      current_angle += change_angle;
      //SET CTMs
      for(int i = 0; i < 9; i++){
        if(current_state == DOWN){
          ctm_array[down[i]] = mat_mat_mul(rotate_y(change_angle), ctm_array[down[i]]);
        }else{
          ctm_array[down[i]] = mat_mat_mul(rotate_y(-change_angle), ctm_array[down[i]]);
        }
      }

      if(current_angle >= PI/2){
        current_angle = 0;
        is_animate = 0;
        //current_state = NONE;
      }

    }else{
      //is_animate = 0;
    }
  

  }else{//is_animation = 0

    if(start_shuffle){
      /*
      int formula[28] = {1, 6, 6, 2, 5, 3, 3, 3, 5, 3, 
                        3, 3, 5, 3, 3, 3, 2, 2, 2, 6, 
                        6, 1, 5, 5, 4, 4, 1, 1};
      */
       

      if(formula[step] == 1){
          turn_cube(1);
        }else if(formula[step] == 2){
          turn_cube(2);
        }else if(formula[step] == 3){
          turn_cube(3);
        }else if(formula[step] == 4){
          turn_cube(4);
        }else if(formula[step] == 5){
          turn_cube(5);
        }else if(formula[step] == 6){
          turn_cube(6);
        }else if(formula[step] == 7){
          turn_cube(7);
        }else if(formula[step] == 8){
          turn_cube(8);
        }else if(formula[step] == 9){
          turn_cube(9);
        }else if(formula[step] == 10){
          turn_cube(10);
        }else if(formula[step] == 11){
          turn_cube(11);
        }else if(formula[step] == 12){
          turn_cube(12);
        }

        if(step == max_step){
          current_state = NONE;
          is_animate = 0;
          start_shuffle = 0;
        }
 
      step++;

    }
  }

  glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("project4");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutSpecialFunc(specialkey);
    glutKeyboardFunc(keyboard);
    
    glutMouseFunc(mouse);//mouse
    //glutMotionFunc(motion);//Motion
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();

    free(vertices);
    free(colors);
    free(normals);
    
    return 0;
}
