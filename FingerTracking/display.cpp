#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <XnCppWrapper.h>
#include <XnVNite.h>
#include "mode.h"
#include "picking.h"
#include "gesture.h"
#include "drawmodel.h"
#include "gesture.h"
#include "hand_history.h"
#include "vertex.h"
#include "vmmodel.h"
#include "display.h"
#include "paint.h"

//resolution
static XnUInt16 g_nXRes, g_nYRes;
static int h=480, w= 800;
static float cursorX, cursorY; 

bool rotateMode = false;
bool stateGrab = false;		//0- not grab, 1 - already in grab
bool static BACK_BUFF;		//display back buffer


/* Select from 3 available modes and process accordingly */
void mode_selection(XnPoint3D* handPointList, hand_h* rhand, hand_h* lhand){

	//-------------------sculpting--------------------------
	if(is_mode(1)){
		//right hand
		checkCursor(1, rhand, lhand); 
		
		//SELECTION
		if(is_state(1)){
			drawPickVMModel();
			processPick(cursorX, cursorY);
			set_state(2);	
		}
		//RENDER
		else {
			draw_hand(handPointList);	//draw every hands
			//show back buffer?
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
	else if(is_mode(2)) {
		checkCursor(2, rhand, lhand); 

		//SELECTION
		if(is_state(1)){
			//update cursor for paint effect
			cursorX = (g_nXRes-getPalm().X)*w/g_nXRes;
			cursorY = getPalm().Y*h/g_nYRes;
			drawPickVMModel();
			processPick(cursorX, cursorY);
			set_state(2);
		}
		//RENDER
		else {
			draw_hand(handPointList);
			if(!BACK_BUFF)
				drawVMModel();
			else drawPickVMModel();

			//back to select other mesh
			set_state(1);
			glutSwapBuffers();
		}
	}

	//------------------ Selection ------------------------
	else if(is_mode(3)) {
		checkCursor(3, rhand, lhand);
 
		if(is_state(1)){
			drawPickVMModel();

			set_state(2);
		}
		else {
			draw_hand(handPointList);
			if(!BACK_BUFF)
				drawVMModel();
			else drawPickVMModel();
			drawVMModel();
			glutSwapBuffers();
		}
		
	}

}


//treat grab as a mouse click. 
void checkCursor(int func, hand_h* rhand, hand_h* lhand){
	
	//right hand
	if(isGrab()) {
		rhand->storeHand(getPalm());		//keep the hand movement history

		//first time grab gesture occurs
		if(!stateGrab) {	
			//adjust with width and height of the screen
			cursorX = (g_nXRes-getPalm().X)*w/g_nXRes;
			cursorY = getPalm().Y*h/g_nYRes;
			set_state(1); 
			stateGrab = true;
			//Beep(750,50);				//play sound

		}
		//still in grab gesture
		else{
			//free hand
			if(func == 1) {
				//select a mesh once
				//we don't need this for painting
				set_state(2);

				if(rotateMode){
					//disableLine(); //disable line effect
					commitScene(rhand->gettranslateX(), rhand->gettranslateY(), rhand->gettranslateZ());
					recalNormal();
				}else{

					//grab group of mesh
					if(sListContain(getSelection()) >= 0 ){
						interpolate(getsList(), rhand->gettranslateX(), 
							rhand->gettranslateY(), rhand->gettranslateZ(), getRotX(), getRotY());
						recalNormal();
					}
					//grab one mesh
					else if(getSelection() > 0 && getSelection() < getFaceListSize()){
						interpolate(getSelection(), rhand->gettranslateX(), 
							rhand->gettranslateY(), rhand->gettranslateZ(), getRotX(), getRotY());
						recalNormal();
					}
				}

			}
			//paint
			else if(func ==2){
				if(rotateMode){
					disableLine(); 
					commitScene(rhand->gettranslateX(), rhand->gettranslateY(), rhand->gettranslateZ());
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
			//if(selection && getSelection() > 0 && getSelection() < getFaceListSize()){
			//	store_selection(getSelection());
			//}

			stateGrab = false; 
			rhand->clearHandList();
			setNullSelection(); //show no mesh response when hand released

			//undo
			if(func == 1) copy_vmmodel(); 
		}
	}

}

//helper
void rotate(){
	rotateMode = !rotateMode;
}

void set_nRes(XnUInt16 XRes, XnUInt16 YRes){
	g_nXRes = XRes;
	g_nYRes = YRes;
}

void set_cursor(float x, float y){
	cursorX = x;
	cursorY = y;
}
void switch_buffer(){
	BACK_BUFF = !BACK_BUFF;
}
bool get_buffer(){
	return BACK_BUFF;
}