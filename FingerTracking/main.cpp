
/* Chantree Chandrapunth
*  Senior Project: Virtual Sculpture 
*
*  Main loop controls the workflow while connecting 
*  the gesture and the graphical component together. 
*  Handle most of the rendering part
*/

#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>
#include <time.h>

#include "picking.h"
#include "gesture.h"
#include "hand_history.h"
#include "window.h"
#include "undo.h"
#include "ui.h"
#include "vertex.h"
#include "vmmodel.h"
#include "drawmodel.h"
#include "paint.h"
#include "svmtrain.h"
#include "Pair.h"
#include "svmpredict.h"
#include "drawbackground.h"

//----------------------------------------------------------------
//							Variable
//----------------------------------------------------------------

#define SelBuffSize 512		//selection buffer 
#define BUFSIZE 1024 


//rendering mode
#define RENDER	1	
#define SELECT	2	

//mode
bool rotateMode = false;

// static //
static int mainWindow;
static int border =6, h=480, w= 800; 
static float cursorX, cursorY; 
static int mode = RENDER; 	
static XnUInt16 g_nXRes, g_nYRes;
static bool BACK_BUFF;	//show back buffer
static XnPoint3D *handPointList;

GLuint selectBuf[BUFSIZE];

//gesture 
#define GESTURE_TO_USE "Click" 
#define MAXPOINT 30000
xn::DepthGenerator *ptr_DepthGen;
xn::Context context;
bool stateGrab = false; //0- not grab, 1 - already in grab


//ui
ui *Master_ui =new ui();


// feature
bool sculpting = true;
bool knife = false;
bool paint = false;
bool selection = false;

//paint
#define checkImageWidth 64
#define checkImageHeight 64

static GLuint texName[6];
static GLubyte checkImage[checkImageWidth][checkImageHeight][4]; //colorId 1
static GLubyte redTex[4];	//2
static GLubyte blueTex[4];	//3
static GLubyte greenTex[4];	//4
static GLubyte yellowTex[4];	//5
static GLubyte whiteTex[4];	//6



//viewport

float zNear;
float zFar; 
GLdouble left; 
GLdouble right; 
GLdouble bottom; 
GLdouble top; 

//---------------------------------------------------------------
//				Keyboard and mouse
//----------------------------------------------------------------


void mouse(int button, int state, int x, int y){

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	//only when the left button is clicked 
	cursorX = x;
	cursorY = y;
	mode = SELECT;
}

void processNormalKeys(unsigned char key, int x, int y){
	
	if(key ==27){			//'esc' to exit
		exit(0);
	}
	else if(key == 104){	//'h' to show handmap or palmpoint
		switchShowHand();
	}
	else if(key == 100){	//'d' to show front buffer or back bufferr
		BACK_BUFF = !BACK_BUFF;
		if(BACK_BUFF) printf("switch buffer to front\n");
		else 	printf("switch buffer to back\n");
	}
	else if(key == 111) {//'o' to train value = open hand
		set_print_training(2);
	}

	else if(key == 112) {// 'p' to train value = close hand
		set_print_training(1);
	}
	else if(key == 49){ //'1' for rotate X
			commitScene(1, 0, 0);
			recalNormal();
	}
	else if(key == 50){ //'2' for rotate y
			commitScene(0,1,0);
			recalNormal();
	}
	else if(key == 51){ //'3' for line effect
			switchLine();
	}
	else
		printf("key: %d\n", key);
}

//treat grab as a mouse click. 
void checkCursor(int func){
	
	if(isGrab()) {
		//keep the movement history
		storeHand(getPalm());

		//first time grab gesture occurs
		if(!stateGrab) {	
			//adjust with width and height of the screen
			cursorX = (g_nXRes-getPalm().X)*w/g_nXRes;
			cursorY = getPalm().Y*h/g_nYRes;
			mode = SELECT; 
			stateGrab = true;
			//Beep(750,50);				//play sound

		}
		//still in grab gesture
		else{
			//free hand
			if(func == 1 && !selection) {
				//select a mesh once
				//we don't need this for painting
				mode = RENDER;	

				if(rotateMode){
					disableLine(); //disable line effect
					commitScene(gettranslateX(), gettranslateY(), gettranslateZ());
					recalNormal();
				}else{

					//grab group of mesh
					if(sListContain(getSelection()) >= 0 ){
						interpolate(getsList(), gettranslateX(), gettranslateY(), gettranslateZ(), getRotX(), getRotY());
						recalNormal();
					}
					//grab one mesh
					else if(getSelection() > 0 && getSelection() < getFaceListSize()){
						interpolate(getSelection(), gettranslateX(), gettranslateY(), gettranslateZ(), getRotX(), getRotY());
						recalNormal();
					}
				}

			}
			//paint
			else if(func ==2 && !selection){
				if(rotateMode){
					disableLine(); 
					commitScene(gettranslateX(), gettranslateY(), gettranslateZ());
					recalNormal();
				}else{

					if(getSelection() >0 && getSelection() < getFaceListSize()){
						//printf("selection ->%d\n", getSelection());
						paintMesh(getSelection(), getBrushColor());
					}		
				}
			}

			//knife
			else if(func ==3){
				
			}
		}
	}
	else{
		//just release
		if(stateGrab){
			//selection list
			if(selection && getSelection() > 0 && getSelection() < getFaceListSize()){
				store_selection(getSelection());
			}

			stateGrab = false; 
			clearHandList();
			setNullSelection(); //show no mesh response when hand released

			//undo
			if(func == 1) copy_vmmodel(); 
			else if(func ==2) {
				//pushMatrix(); //undo rotate
				copy_vmmodel();
			}
		}
	}

}
//------------------------------------------------------------------
//								Texture & UI
//------------------------------------------------------------------

void makeTexImage(){
	int i, j, c;

	//check board
	for(i=0; i< 64; i++){
		for(j=0; j< 64; j++){
			c = (((i&0x8)==0) ^ ((j&0x8))==0)*255;		
			checkImage[i][j][0] = (GLubyte) c;
			checkImage[i][j][1] = (GLubyte) c;
			checkImage[i][j][2] = (GLubyte) c;
			checkImage[i][j][3] = (GLubyte) 255;
		}	
	}

	//red
	redTex[0] = 255; redTex[1] = 0; redTex[2] = 0; redTex[3] = 255;

	//blue
	blueTex[0] = 0; blueTex[1] = 0; blueTex[2] = 255; blueTex[3] = 255;

	//green
	greenTex[0] = 0; greenTex[1] = 255; greenTex[2] = 0; greenTex[3] = 255;

	//yellow 
	yellowTex[0] =255; yellowTex[1] = 255; yellowTex[2] = 0; yellowTex[3] = 255;

	//white
	whiteTex[0] = 255; whiteTex[1] = 255; whiteTex[2] = 255; whiteTex[3] = 255;

}
void UIhandler(){
	
	Master_ui->check_click(convertX(getPalm().X), convertY(getPalm().Y));
}

//------------------------------------------------------------------
//								display
//------------------------------------------------------------------

void display(){

	if(glutGetWindow()!=mainWindow)  
		glutSetWindow(mainWindow);
	if(handPointList == NULL){
		printf("error. can't allocate memory for handPointList");
	}

	glutSetWindow(mainWindow);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	draw_background();

	glLoadIdentity();

	UIhandler(); //check ui touch

	//-------------------sculpting--------------------------
	
	if(sculpting){
		//check grabing, store palm
		checkCursor(1); 
		
		if(mode == SELECT){
			drawPickVMModel();
			processPick(cursorX, cursorY);
			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF){
				drawVMModel();
			}
			else{ 
				drawPickVMModel();
			}
			glutSwapBuffers();
		}

	}
	//-------------------paint----------------------------
	else if(paint) {
		checkCursor(2); 
		if(mode == SELECT){
			//update cursor
			cursorX = (g_nXRes-getPalm().X)*w/g_nXRes;
			cursorY = getPalm().Y*h/g_nYRes;
			drawPickVMModel();
			processPick(cursorX, cursorY);
			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF)
				drawVMModel();
			else drawPickVMModel();

			//back to select other mesh
			mode = SELECT;
			
			glutSwapBuffers();
		}
	}
	else if(knife) {
		checkCursor(3);
 
		if(mode == SELECT){
			drawPickVMModel();

			mode = RENDER;
		}
		else {
			drawHand(handPointList);
			if(!BACK_BUFF)
				drawVMModel();
			else drawPickVMModel();
			glutSwapBuffers();
		}
		
	}

	context.WaitAndUpdateAll();
	
	glutSetWindow(mainWindow); //set current GLUT window before rendering
	glFlush();
	
}

//-------------------------------------------------------------------------
//									Reshape
//-------------------------------------------------------------------------
void reshape(int w1, int h1){

	glutSetWindow(mainWindow); 

	h = h1; 
	w = w1; 
	if(h1 ==0) h=1; 
	glViewport(0, 0, w, h); 

	float adjbottom = bottom;
	float adjtop = top;
	float adjright = right;
	float adjleft = left;

	/*
	float aspect= (float)w/h;
	printf("asp: %f\n", aspect);
	if ( aspect < 1.0 ) { // window taller than wide
     adjbottom /= aspect;
     adjtop /= aspect;
   } else {
     adjleft *= aspect;
     adjright *= aspect;
   }
   */

	//PROJECTION: set window coordinate
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 

	XnMapOutputMode mode;
	(*ptr_DepthGen).GetMapOutputMode(mode);

	g_nXRes = mode.nXRes;
	g_nYRes = mode.nYRes;

	//set the clipping volume corresponding to the viewport
	//left, right, buttom, top
	glOrtho(adjleft, adjright, adjbottom, adjtop, zNear, zFar);

	//eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz
	gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

	printf("l: %f, r:%f, bot: %f, top: %f, zN: %f, zF: %f\n", adjleft, adjright, adjbottom, adjtop, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//----------------------------------------------------------------
//								INIT
//----------------------------------------------------------------
void initTex(void){
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_DEPTH_TEST);
	makeTexImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(6, texName);
	printf("generate texture: %d, %d, %d, %d, %d, %d\n", texName[0], texName[1], texName[2], texName[3], texName[4], texName[5]);


	//check texture
	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);


	//plane color texture
	for(int i=1; i< 6; i++){
		glBindTexture(GL_TEXTURE_2D, texName[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		if(i==1) 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, redTex);
		if(i==2)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, greenTex);
		if(i==3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueTex);
		if(i==4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, yellowTex);
		if(i==5) 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteTex);
	}
}


void initRender(){
	GLfloat lmodel_ambient[] = {0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_specular[] = {0.8, 0.8, 0.8, 1.0};
	GLfloat diffuseMaterial[4] = {0.4, 0.4, 0.4, 1.0};
	GLfloat diffuse[] = {1.0,1.0,1.0,1.0};
	GLfloat light_position[] = {1.0, 2.0, 1, 1.0};
	GLfloat light_position1[] = {-1.0, 2.0, 1, 1.0};
	GLfloat light_position2[] = {0.0, -0.2, 8, 1.0};

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

	//light0-1
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);	
	

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	//new
	import_vm();
	copy_vmmodel();
	findBoundingSphere();

	handPointList = new XnPoint3D[MAXPOINT];
	
	glEnable(GL_NORMALIZE);			//automatically rescale normal when transform the surface

}

//-----------------push menu-------------------
//sculpting
void option1(){
	printf("Ready to sculpt?\n");
	sculpting = true;
	knife = false;
	paint = false;
	Master_ui->remove_menu();
}
//paint brush
void option2(){
	printf("paint\n");
	sculpting = false;
	knife = false;
	paint = true;
	Master_ui->remove_menu();

	float width = right -left;
	float height = top -bottom; 
	float off = height/30;

	Master_ui->add_button("red", left+ width/15, top-height/10, width/10, height/10-off, setRed);	//red
	Master_ui->add_button("green", left+ width/15, top-height*2/10, width/10, height/10-off, setGreen);	//green
	Master_ui->add_button("blue", left+ width/15, top-height*3/10, width/10, height/10-off, setBlue);	//blue
	Master_ui->add_button("yellow", left+ width/15, top-height*4/10, width/10, height/10-off, setYellow);	//yellow
	Master_ui->add_button("white", left+ width/15, top-height*5/10, width/10, height/10-off, setWhite);	//white
}
//knife?
void option3(){
	printf("Knife\n");
	sculpting = false;
	knife = true;
	paint = false;
	Master_ui->remove_menu();

}

void up(){
	if(sculpting){
		upEffect();
	}
	else{
		upBrush();
	}
}

void down(){
	if(sculpting){
		downEffect();
	}
	else{
		downBrush();
	}
}

//reload model
void reload(){
	import_vm();
}
void selectionMode(){
	selection = !selection;

	if(!selection){
		//clearSelectionList();
	}
}

void rotate(){
	rotateMode = !rotateMode;
}

//FIXME: should hide the menu button once it's click
void push_menu(){
	//draw panel 
	Master_ui->activate_menu = true; //addpanel
	printf("pushing menu\n");

	float width = right -left;
	float height = top -bottom; 
	float off = width/20;

	Master_ui->add_button("Sculpt", left+ width/5, bottom+height/3, width/5-off, height/3, option1);
	Master_ui->add_button("Paint", left+ width*2/5, bottom+height/3, width/5-off, height/3, option2);
	Master_ui->add_button("Slice", left+ width*3/5, bottom+height/3, width/5-off, height/3, option3);
}

//all ui in here
void uiInit(){

	vertex c = getCenter();
	float diam = getDiam();

	zNear = -5;
    zFar = 100;
	left = c.x - diam;
    right = c.x + diam;
    bottom = c.y - diam;
    top = c.y + diam;

	//main menu button
	Master_ui->add_button("Menu", left+(right-left)/15, bottom+0.5, 0.5, 0.3, push_menu);
	Master_ui->add_button("-", left+(right-left)/15, bottom+0.8, 0.5, 0.3, down);
	Master_ui->add_button("+", left+(right-left)/15, bottom+1.1, 0.5, 0.3, up);

	Master_ui->add_button("reset", right-(right-left)/5, bottom+0.5, 0.5, 0.3, reload);	//if remove, fix ui.cpp (count)
	//Master_ui->add_button("select", right-(right-left)/5, bottom+0.8, 0.5, 0.3, selectionMode);
	Master_ui->add_button("rotate", right-(right-left)/5, bottom+0.8, 0.5, 0.3, rotate);
	Master_ui->add_button("undo", right-(right-left)/5, bottom+1.1, 0.5, 0.3, undo_vmmodel);
}


void kinectInit(){

	//debug to gesture log file "depthmap.txt"
	enableDebugGesture();

	// Initialize context object
	XnStatus nval = context.Init();
	CHECK_RC(nval, "Initialize context");

	// Create the gesture, hand, depth object
	ptr_DepthGen = getDepthGenerator(context);
	HandsGenerator* ptr_HandsGen = getHandGenerator(context);
	GestureGenerator* ptr_GestureGen = getGestureGenerator(context);


	//Register to callbacks
	XnCallbackHandle h1, h2;
	nval = (*ptr_GestureGen).RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, NULL, h1);
	CHECK_RC(nval, "Register gesture");
	nval = (*ptr_HandsGen).RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, NULL, h2);
	CHECK_RC(nval, "Register hands");

	// Tell the context objects to start generating data
	nval = context.StartGeneratingAll();
	CHECK_RC(nval, "Start Generating All Data");

	// Calibration
	nval = (*ptr_GestureGen).AddGesture(GESTURE_TO_USE, NULL);									
	CHECK_RC(nval, "Add gesture");

}

void initSVM(){
	
	char raw_training_set []= "training.txt";
	char training_model [] = "training_mod.txt";
	//char test_set [] = "svm_testing.txt";
	//char predicted_result [] = "svm_result.out";
	
	svm_train(raw_training_set, training_model);
	init_predict(training_model);

}
void wait ( int seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}


int main (int argc, char **argv){
	
	
	kinectInit();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(w,h);
	mainWindow = glutCreateWindow("picking back buffer");

	glutKeyboardFunc(processNormalKeys);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);


	initRender();
	//initTex();
	uiInit();
	initSVM();
	glutIdleFunc(display);	//enable GLUI window to take advantage of idle event

	createGLUTMenus();
	glutMainLoop();

	context.Shutdown();
	return(0);
	
	
	/*
	char raw_training_set []= "a1a.train.train";
	char training_model [] = "a1a.mod.txt";
	char test_set [] = "a1a.test.test";
	char predicted_result [] = "a1a.result.out";
	svm_train(raw_training_set, training_model);
	svm_predict(test_set, training_model, predicted_result);
	*/

	/*
	char raw_training_set []= "training.txt";
	char training_model [] = "training.mod.txt";
	char test_set [] = "testing.txt";
	char predicted_result [] = "result.out";
	svm_train(raw_training_set, training_model);
	svm_predict(test_set, training_model, predicted_result);
	*/
	
	wait(100);
	


}