
//-----------------------------------------------
//			Model data structure
//-----------------------------------------------

//A vetex has its x,y,z coordinate
typedef struct vertex_s
{
	float X,Y,Z;
} vertex_t;

//list of all the verteices
typedef struct point_s
{
	int nVertexs; 
	vertex_t pPoints[12]; 
} point_t;

//A quad contains 4 points' id and its normal
typedef struct polygon_s
{
	int p[4];
	vertex_t normal;	//surface normal
} polygon_t;

//list of all quads
typedef struct model_s
{
	int nPolygons;
	polygon_t *pList;
	
} model_t;


//-------------------------------------------------------
//						extern
//-------------------------------------------------------
extern model_t sampleModel;
extern point_t samplePoint;


//--------------------------------------------------------
//						function
//--------------------------------------------------------
void ImportModel();
void ExportModel();

void LoadModel(point_t* point, model_t* model);
void drawMe (model_t *model, point_t* poly);
void drawPickMe(model_t* model, point_t* poly);
void FreeModel (model_t* model);
void calculateNormal(point_t* point, model_t* model);

//translate
void translatePoly(model_t* model, int id, point_t* vertexlist,float transx, float transy);

//---------------------------------------------------------
//							geter
//---------------------------------------------------------
int getnPoint();
int getnPoly();
float* getPoint();
int* getPoly();