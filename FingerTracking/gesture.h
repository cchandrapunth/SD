
//OpenNI and NITE setting for the scene

using namespace xn; 

void enableDebugGesture();

//-----------------------------------------
//			error checking
//-----------------------------------------
#define CHECK_RC(rc, what)										\
if(rc != XN_STATUS_OK){											\
	printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
	exit(1);													\
}			

//register the generator 
xn::DepthGenerator* getDepthGenerator(Context);
xn::HandsGenerator* getHandGenerator(Context);
xn::GestureGenerator* getGestureGenerator(Context);


//callback to initialize gesture
void XN_CALLBACK_TYPE Gesture_Recognized(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition,void* pCookie);

void XN_CALLBACK_TYPE Gesture_Process(GestureGenerator& generator, const XnChar* strGesture, 
	const XnPoint3D* pPosition, XnFloat fProgress,void* pCookie);

//callback to initialize hand
void XN_CALLBACK_TYPE Hand_Create(HandsGenerator& generator,XnUserID nId,const XnPoint3D* pPosition,
		XnFloat fTime, void* pCookie);

void XN_CALLBACK_TYPE Hand_Update(HandsGenerator& generator,XnUserID nId, const XnPoint3D* pPosition, 
	XnFloat fTime, void* pCookie);

void XN_CALLBACK_TYPE Hand_Destroy(HandsGenerator& generator,XnUserID nId, XnFloat fTime,void* pCookie);


//read hand area 
void draw_hand(XnPoint3D* handPointList);
int draw_map(XnPoint3D* handPointList, XnPoint3D m_palm, int id);
void predict_gesture(XnPoint3D* handPointList, XnPoint3D palm, int n, int id);

bool getEdge(XnPoint3D* List, int nNumberOfPoints, XnPoint3D palm);

//helper
bool isGrab();
XnPoint3D getPalm();
bool isLGrab();
XnPoint3D getLPalm();

void switchShowHand();
bool estimateGrab(
	XnPoint3D* list, 
	int n, XnPoint3D* highest, 
	XnPoint3D* lowest, 
	XnPoint3D* leftmost, 
	XnPoint3D* rightmost, 
	XnPoint3D palm);

bool find_finger(XnPoint3D* List, int nNumberOfPoints, XnPoint3D palm);
float findAngle(float x1, float y1, float x2, float y2, float x3, float y3);

float convertX(float x);
float convertY(float y);
float convertYcursor(float y);

void set_print_training(int i);



///helper
bool hasTwoHands();
float dis(float x1, float y1, float x2, float y2);