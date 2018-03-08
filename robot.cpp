
#include "include/Angel.h"
#include <iostream>
#include <assert.h>
#include <cstring>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;
const GLfloat ThetaDelta = 5.0;
const GLfloat Pi = 3.141592653589793;

// Shader transformation matrices
mat4 model;
mat4 view;
GLuint uniModel, uniProjection, uniView;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  Quit = 4;

// ROBOT_MODE
enum {
    FREE = 0,
    FETCH = 1
};
int ROBOT_MODE;

// VIEW_MODE
enum {
    TOP = 0,
    SIDE = 1,
};
int VIEW_MODE = SIDE;

// position of sphere
int old_x, old_y, old_z, new_x, new_y, new_z;
// equals old xyz first, then new syz, then 000
int current_target_x, current_target_y, current_target_z;
//----------------------------------------------------------------------------

int Index = 0;

inline float degree_to_radian(float degree) {
    return degree * Pi / 180;
}

inline float radian_to_degree(float radian) {
    return radian * 180 / Pi;
}

void quad( int a, int b, int c, int d ) {
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

// generate a cube
void colorcube() {
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base() {
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}
void move_base(int value) {
    cout << "executing move_base()\n";
    // only execute once move, and call many times
    // calculate the direction: clock-wise (+1) or not (-1)
    int base_direction = current_target_z > 0 ? 1 : -1;
    // easy to make mistakes here: current_target_x, current_target_z axis
    // calculate the radians in current_target_x-current_target_z axis
    float base_theta = degree_to_radian(180 - Theta[Base]);
    // calculate base vector (point to the direction which arms can move)
    vec2 base_vector = vec2(cos(base_theta), sin(base_theta));
    // vector of sphere in current_target_x-current_target_z axis
    vec2 sphere_vector = vec2(current_target_x, current_target_z);
    // alpha is the radian of the difference between the two vectors
    float alpha = acos(dot(base_vector, sphere_vector) / length(sphere_vector)); // note length(base_vector) == 1
    cout << "Distance angle: " << radian_to_degree(alpha) << endl;
    assert(alpha >= 0);
    if(alpha > degree_to_radian(ThetaDelta) / 2) {
        // need to rotate
        // TODO:
        Theta[Base] += base_direction * ThetaDelta;
        glutPostRedisplay();
        // delay for a period and call myself again
        // TODO:
        glutTimerFunc(200, move_base, 0);
    } else {
        // no need to rotate, set the flag
        // TODO:
    }
}

//----------------------------------------------------------------------------

void upper_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void lower_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void display( void ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // TODO: figure out the middle point of the objects problem
    if(VIEW_MODE == TOP) {
        view = LookAt(vec4(1, 1, 5, 1), vec4(1, 0, 5, 1), vec4(0, 0, -1, 0));
    } else {
        view = mat4(1);
    }
    glUniformMatrix4fv(uniView, 1, GL_TRUE, view); 

    // Accumulate uniModel Matrix as we traverse the tree
    model = RotateY(Theta[Base] );
    base();

    model *= ( Translate(0.0, BASE_HEIGHT, 0.0) *
		    RotateZ(Theta[LowerArm]) );
    lower_arm();

    model *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
		    RotateZ(Theta[UpperArm]) );
    upper_arm();

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init( void ) {
    colorcube();
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArraysAPPLE( 1, &vao );
    glBindVertexArrayAPPLE( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		  NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );
    
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    uniModel = glGetUniformLocation( program, "uniModel" );
    uniView = glGetUniformLocation(program, "uniView");
    uniProjection = glGetUniformLocation( program, "uniProjection" );

    glEnable( GL_DEPTH );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int current_target_x, int y ) {

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
	// Incrase the joint angle
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
	// Decrase the joint angle
    }

}

void onSpecialKeyPressed(int key, int current_target_x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            Theta[Axis] += ThetaDelta;
            if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
            break;
        case GLUT_KEY_RIGHT:
            Theta[Axis] -= ThetaDelta;
            if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu( int option ) {
    if ( option == Quit ) {
	exit( EXIT_SUCCESS );
    }
    else {
	Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape( int width, int height ) {
    glViewport( 0, 0, width, height );

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width) / height;

    if ( aspect > 1.0 ) {
	left *= aspect;
	right *= aspect;
    }
    else {
	bottom /= aspect;
	top /= aspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( uniProjection, 1, GL_TRUE, projection );

    model = mat4( 1.0 );  // An Identity matrix
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int current_target_x, int y ) {
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

int main( int argc, char **argv ) {
    glutInit( &argc, argv );
    // parse argv
    cout << "Number of args: " << argc << endl;
    if(argc == 8) {
        ROBOT_MODE = FETCH;
        cout << "Enter Fetch mode" << endl;

        // assign sphere position
        old_x = atoi(argv[1]);
        old_y = atoi(argv[2]);
        old_z = atoi(argv[3]);
        new_x = atoi(argv[4]);
        new_y = atoi(argv[5]);
        new_z = atoi(argv[6]);
        current_target_x = old_x;
        current_target_y = old_y;
        current_target_z = old_z;

        // perspective
        if(strcmp("-tv", argv[7]) == 0) {
            VIEW_MODE = TOP;
            cout << "Use top view\n";
        } else {
            VIEW_MODE = SIDE;
            cout << "Use side view\n";
        }
    } else {
        ROBOT_MODE = FREE;
        cout << "Enter Free mode" << endl;
    }
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "robot" );

    glewExperimental = GL_TRUE; 
    glewInit();
    init();

    glutDisplayFunc( display );
    // triggered when window is reshaped
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc(onSpecialKeyPressed);
    glutMouseFunc( mouse );

    glutCreateMenu( menu );
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry( "base", Base );
    glutAddMenuEntry( "lower arm", LowerArm );
    glutAddMenuEntry( "upper arm", UpperArm );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    if(ROBOT_MODE == FETCH) {
        glutTimerFunc(1000, move_base, 0);
    }

    glutMainLoop();
    return 0;
}
