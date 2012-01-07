#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>

#include "picking.h";
#include "model.h";
#include "undo.h";

static GLint model_display_list;


void processPick(int cursorX, int cursorY){
	GLint viewport[4];
	GLubyte pixel[3];

	//ask for value of the viewport
	glGetIntegerv(GL_VIEWPORT, viewport);

	//read pixel under the curser
	glReadPixels(cursorX, viewport[3]-cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*) pixel);

	printf("%d %d %d\n", pixel[0], pixel[1], pixel[2]);

	if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0){
	  printf ("You picked the 1st rect");
	  pickMe = 0;
	}
	else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0){
	  printf ("You picked the 2nd rect");
	  pickMe = 1;
	}
	else if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255){
	  printf ("You picked the 3rd rect");
	  pickMe = 2;
	}
	else if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 0){
	  printf ("You picked the 4rd rect");
	  pickMe = 3;
	}
	else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 255){
	  printf ("You picked the 5rd rect");
	  pickMe = 4;
	}
	else if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255){
	  printf ("You picked the 6rd rect");
	  pickMe = 5;
	}
	else if (pixel[0] == 130 && pixel[1] == 0 && pixel[2] == 130){
	  printf ("You picked the 7rd rect");
	  pickMe = 6;
	}
	else if (pixel[0] == 0 && pixel[1] == 130 && pixel[2] == 130){
	  printf ("You picked the 8rd rect");
	  pickMe = 7;
	}
	else{
	   printf("You didn't click any rect!");
	   pickMe = -1;
	 }
	printf ("\n");

}

int getSelection(){
	return pickMe;
}

void picked(GLuint name, int sw){
	printf("my name = %d in %d\n", name, sw);
}