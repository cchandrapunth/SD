typedef void (*cb_function)();
class ui_button
{
public:

	bool currently_inside;
	int user_id;
	int w, h, x, y;
	bool execute;
	bool activate;
	int wait;	//hand hovering time
	cb_function callback;

    int  hand_down_handler( int local_x, int local_y );
    int  hand_up_handler( int local_x, int local_y, bool inside );

    void draw();
    void draw_pressed( void );
    void draw_text( int sunken );

	void reactivate();
	/**
	Create a new button.
	 @param parent The panel our object is inside; or the main GLUI object.
	 @param name The text inside the button.
	 @param id Optional ID number, to pass to the optional callback function.
	 @param callback Optional callback function, taking either the int ID or control.
	*/

	ui_button(const char *name, long id, int begin_x, int begin_y, int width, int height, cb_function cb);
	ui_button(const char *name, long id, int begin_x, int begin_y, cb_function cb);
	ui_button(void){ common_init(); };

private:
    void common_init(void) {
        h  = 60;
        w  = 100;
        //alignment    = GLUI_ALIGN_CENTER;

	}

};