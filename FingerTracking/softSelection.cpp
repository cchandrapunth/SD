#include "stdafx.h"
#include <math.h>
#include "model.h"
#include "softSelection.h"

#define e 2.71828

point_t* vlist; 
float* plist;
float maxEffect = 0.5;
static float s = 0.3;

void translatePoint(int id, float mult, float transx, float transy){
		float prevx = vlist->pPoints[id].X;
		float prevy = vlist->pPoints[id].Y;

		vlist->pPoints[id].X = prevx+(mult*transx);
		vlist->pPoints[id].Y = prevy+(mult*transy);

		printf("id: %d, mult: %f, tranx: %f, transy: %f\n", id, mult, transx, transy);
		//index indecates points 
		plist[id*3] = prevx+(mult*transx);	//x pointdinate
		plist[id*3+1] = prevy+(mult*transy); //y pointdinate
}

//assign max distance = 0.5
void linearfunc(float xc, float yc, float transx, float transy){
	int n = vlist->nVertexs;
	
	for(int i=0; i<n; i++){
		float dis = sqrt(pow(vlist->pPoints[i].X -xc, 2)+ pow(vlist->pPoints[i].Y -yc, 2));
		if(dis < maxEffect){
			double param = (maxEffect-dis)/maxEffect;
			translatePoint(i,pow(param, 2), transx/5, transy/5);
		}
		/*
		float x = vlist->pPoints[i].X; 
		float y = vlist->pPoints[i].Y;
		double func = (0.3989/s)*pow(e, -(double)((pow((x-xc),2) + pow((y-yc), 2))/(2*pow(s,2))));

		printf("func: %f\n", func);
		translatePoint(i,func, transx, transy);
		*/
	}

}


void softSelect(model_t* model, int id, point_t* vertextlist,float transx, float transy, float* pointt){

	vlist = vertextlist;
	plist = pointt;

	//acquire the mesh
	polygon_t mesh= model->pList[id];
	
	//get its vertex id
	int* targets = mesh.p;
	
	//old stuff
	//find vertex center of the mass (estimated)
	//float xCenter = (vlist->pPoints[targets[0]].X + vlist->pPoints[targets[1]].X+ vlist->pPoints[targets[2]].X)/3;
	//float yCenter = (vlist->pPoints[targets[0]].Y + vlist->pPoints[targets[1]].Y+ vlist->pPoints[targets[2]].Y)/3;

	//linearfunc(xCenter, yCenter, transx, transy);

	/* Just move the one
	for(int i=0; i<3; i++){
		float prevx = vlist->pPoints[targets[i]].X;
		float prevy = vlist->pPoints[targets[i]].Y;
		vlist->pPoints[targets[i]].X = transx/100+ prevx;
		vlist->pPoints[targets[i]].Y = transy/100+ prevy;

		printf("id: %d, tranx: %f, transy: %f\n", id, transx, transy);
		//index indecates points 
		plist[targets[i]*3] = transx/100+ prevx;//x pointdinate
		plist[targets[i]*3+1] = transy/100+ prevy; //y pointdinate
	}
	*/

	//normal-based
	vertex_t norm = mesh.normal;
	float normx = abs(norm.X);
	float normy = abs(norm.Y);
	float normz = abs(norm.Z);


		//use tranx to control 
	if(normx > normy){

		for(int i=0; i<3; i++){
		float prevx = vlist->pPoints[targets[i]].X;
		float prevy = vlist->pPoints[targets[i]].Y;
		float prevz = vlist->pPoints[targets[i]].Z;

		vlist->pPoints[targets[i]].X = (transx/100)*abs(norm.X)+ prevx;
		vlist->pPoints[targets[i]].Y = (transx/100)*abs(norm.Y)+ prevy;
		vlist->pPoints[targets[i]].Z = (transx/100)*abs(norm.Z)+ prevz;

		printf("id: %d, tranx: %f\n", id, transx);
		//index indecates points 
		plist[targets[i]*3] = transx/100*abs(norm.X)+ prevx;//x 
		plist[targets[i]*3+1] = transx/100*abs(norm.Y)+ prevy; //y 
		plist[targets[i]*3+2] = transx/100*abs(norm.Z)+ prevz; //z 
		}
	}else{
	 for(int i=0; i<3; i++){
		float prevx = vlist->pPoints[targets[i]].X;
		float prevy = vlist->pPoints[targets[i]].Y;
		float prevz = vlist->pPoints[targets[i]].Z;

		vlist->pPoints[targets[i]].X = (transy/100)*abs(norm.X)+ prevx;
		vlist->pPoints[targets[i]].Y = (transy/100)*abs(norm.Y)+ prevy;
		vlist->pPoints[targets[i]].Z = (transy/100)*abs(norm.Z)+ prevz;

		printf("id: %d, tranx: %f\n", id, transx);
		//index indecates points 
		plist[targets[i]*3] = transy/100*abs(norm.X)+ prevx;//x 
		plist[targets[i]*3+1] = transy/100*abs(norm.Y)+ prevy; //y 
		plist[targets[i]*3+2] = transy/100*abs(norm.Z)+ prevz; //z 
	 }
	}
	
}

