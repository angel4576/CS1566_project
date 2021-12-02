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

GLuint ctm_location;
GLuint model_view_location;
GLuint projection_location;

mat4 ctm;
mat4 ctm2;
mat4* ctm_array;
mat4 model_view = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
mat4 projection = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};;

//eye at up
vec4 eye = {1, 0.8, 1, 1};//vrp
vec4 at = {0, 0, 0, 1};//set(0, 0, -100, 1);
vec4 up = {0, 1, 0, 0};//set(0, 1, 0, 0);//y-axis

//vertices array
vec4 *vertices;
vec4 *colors;

int num_vertices;
int one_cube_num = 36+96;//132

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

mat4 identity(){
  mat4 i = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
  return i;
}

vec4 set(float a, float b, float c, float d){
  vec4 v = {a,b,c,d};
  return v;
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

void translate_cube(){
  edge_length = size + 2*(length-size);
  //translate to the origin
  mat4 t = trans_m(-size/2, -size/2, 2*(length-size)+3*size/2);
  for(int i = 0; i < one_cube_num; i++){
    vertices[i] = mat_vec_mul(t, vertices[i]);
  }
}

void set_sphere_vertex(vec4 *v){    
    int j = 0;
    int i = 0;
    int count = 36*6;
    
    v[0] = set(r,0,0,1);//on the x axis
    vec4 temp = v[0];
    
    
    //upper half of the sphere
    while(j < 8){
    
      vec4 up = mat_vec_mul(rotate_z(PI/18), temp);//3rd
      for(; i < count; i+=6){
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
    for(; i < num_vertices/2; i+= 3){
      v[i] = top_st;
      v[i+1] = mat_vec_mul(rotate_y((PI/18)), v[i]);
      v[i+2] = set(0,r,0,1);//tip
      
      top_st = v[i+1];
    }
    
    count+= 36*3;
    
    //lower part 
    vec4 temp_2 = set(r,0,0,1);
    
    int j_2 = 0;
    while(j_2 < 8){
    
      vec4 down = mat_vec_mul(rotate_z(-PI/18), temp_2);//3rd
      for(; i < count; i+=6){
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
    for(; i < num_vertices; i+= 3){
      v[i] = bottom_st;
      v[i+1] = mat_vec_mul(rotate_y((-PI/18)), v[i]);
      v[i+2] = set(0,-r,0,1);//tip
      
      bottom_st = v[i+1];
    }
    
    
    
    
    
    
}

void set_sphere_color(vec4 *c){
    float ran_x;
    float ran_y;// = (float)rand()/ (float)RAND_MAX;
    float ran_z; //= (float)rand()/ (float)RAND_MAX;
    
    //printf("%f", ran);
    
    for(int i = 0; i < num_vertices; i+=3){
      ran_x = (float)rand()/ (float)RAND_MAX;
      ran_y = (float)rand()/ (float)RAND_MAX;
      ran_z = (float)rand()/ (float)RAND_MAX;
    
    
      c[i] = set(ran_x,ran_y,ran_z,1);
      c[i+1] = set(ran_x,ran_y,ran_z,1);
      c[i+2] = set(ran_x,ran_y,ran_z,1);
    
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


void init(void)
{   
  int input = 2;
  //initialize array
  if(input == 1){//sphere
    num_vertices = 36*6*16 + 36*3*2;
    vertices = (vec4*)malloc(sizeof(vec4)*num_vertices);
    colors = (vec4*)malloc(sizeof(vec4)*num_vertices);

    set_sphere_vertex(vertices);
    set_sphere_color(colors);
  
  }else if(input ==2){
    //cube
    num_vertices = 27*one_cube_num;//cube ver_num
    vertices = (vec4*)malloc(sizeof(vec4)*num_vertices);
    colors = (vec4*)malloc(sizeof(vec4)*num_vertices);
    
    ctm_array = (mat4*)malloc(sizeof(mat4)*27);
    for(int i = 0; i < 27; i++){
      ctm_array[i] = identity();
    }

    //set model view
    model_view = look_at(eye, at, up);
    //set frustrum
    projection = frustum(-0.25, 0.25, -0.25, 0.25, -0.5, -10);//left, right, bottom, top, near, far
    //set one cube vertex
    set_cube_vertex(vertices);
    translate_cube();

    build_first_layer();
    build_two_layers();

    //set all cubes colors
    for(int i = 0; i < 27; i++){
      set_cube_color(colors, i*one_cube_num);//pass the start vertex of each cube
    }
    
    //print color for debug
    /*
    for(int i = 0; i < num_vertices; i++){
      printf("%d ", i);
      print_vec(colors[i]);
    }
    */
    printf("edge length: %f\n", edge_length);
    
    for(int i = 0; i < num_vertices; i++){
      printf("%d ", i);
      print_vec(vertices[i]);
    }
    
  }
   
  
  
    
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*num_vertices+ sizeof(vec4)*num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4)*num_vertices, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4)*num_vertices, sizeof(vec4)*num_vertices, colors);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4)*num_vertices));

    ctm_location = glGetUniformLocation(program, "ctm");//ctm
    ctm = identity();//identity matrix
    ctm2 = identity();

    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");

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
    
    //CTM
    
    for(int i = 0; i < 27; i++){
      glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *) &ctm_array[i]);//send  matrices  into  the  graphic  pipeline 
      glDrawArrays(GL_TRIANGLES, i*one_cube_num, one_cube_num);
      //glDrawArrays(GL_TRIANGLES, one_cube_num, 2*one_cube_num);
      
    }

    glutSwapBuffers();
}

float calculate_z(float x, float y){
  return (float)sqrt(1 - pow(x,2) - pow(y, 2));
}

void mouse(int button, int state, int x, int y){
  //printf("button: %d\n", button);
  float ratio = 1.02;
  mat4 larger = sca_m(ratio,ratio,ratio);
  mat4 smaller = sca_m(1.0/ratio,1.0/ratio,1.0/ratio);
  if(button == 3){
      ctm = mat_mat_mul(larger, ctm);
    
    
  }else if(button == 4){
    ctm = mat_mat_mul(smaller, ctm);
  
  }else if(button == GLUT_LEFT_BUTTON){
   
     init_x = change_x(x);
     init_y = change_y(y);
     init_z = calculate_z(init_x, init_y);
    
     if(isnan(init_z)){
       init_z = 0;
       //return;
     }
    
    /*
    printf("click: %d %d\n", x, y);
    printf("after change x: %f\n", init_x);
    printf("after change y: %f\n", init_y);
    printf("after change z: %f\n", init_z);
    */
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

void keyboard(unsigned char key, int mousex, int mousey)
{
    if(key == 'q'){
    	glutLeaveMainLoop();
    }

    if(key == 'f'){//front
      //ctm = mat_mat_mul(rotate_z(PI/8), ctm);
      for(int i = 0; i < 9; i++){
        ctm_array[i] = mat_mat_mul(rotate_z(PI/8), ctm_array[i]);
      }
      /*
      for(int i = 0; i < 27; i++){
        print_mat(ctm_array[i]);
      }
      */
    }

    if(key == 'u'){//up
      for(int i = 0; i < 27; i++){
        if(i == 5 || i == 3 || i == 6 ||
          i == 14 || i ==12 || i == 15 ||
          i == 23 || i == 21 || i == 24){
            ctm_array[i] = mat_mat_mul(rotate_y(PI/8), ctm_array[i]);


        }
      }
    }
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, 512, 512);
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
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);//mouse
    glutMotionFunc(motion);//Motion
    glutReshapeFunc(reshape);
    glutMainLoop();

    free(vertices);
    free(colors);
    
    return 0;
}
