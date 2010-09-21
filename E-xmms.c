/* 
 E-xmms ver 1.0- A small xmms Epplet 
 Evan Songer <esonger@clarku.edu> 
 This is a GPL Licensed program, you know the deal. 
 --------------------------------------------------
 E-xmms ver 1.2, 1.3, 1.3.1
 Phil Warner <locutus@xmission.com>
 (see above GPL comment)
 
 With modifications from Matt DeLuco <duke@spacebox.net>

 Also: thanks to the icon creaters who submitted their xmms icons to xmms.org

*/

#include <xmms/xmmsctrl.h> 
#include <sys/resource.h>
#include <epplet.h>
#include <string.h>

#define XMMS_PROG "xmms"

#define COUNT_UP 0
#define COUNT_DOWN 1
#define TRACK 2
#define MAX_LEN 9

// the following will disappear when they become configurable
#define PLAY 'p'
#define STOP 's'
#define VOL_UP 'k'
#define VOL_DOWN 'j'
#define NEXT 'l'
#define PREV 'h'
#define EJECT 'e'
#define PAUSE 'x'
#define TITLE 't'

// hard code the number of xmms icons we'll use
// sometime in the future, we can read the directory before setting this.
// and/or we can let the user define another directory.
#define NUMICONS 36

// in order of appearance
Epplet_gadget label, label2, xmms_config, xmms_b, close_b, play_b, pause_b, eject_b, prev_b, next_b, stop_b, vslider, play_label, hbar; 
Window config;
 
extern void Epplet_redraw(void);
 
char *xmmsprog = XMMS_PROG;
int mp_pid = 0;
int xmms_session = 0;
int len = 1, pos = 1, newpos = 1;
int counter = 0; /* counter for the playlist title */
char icon[1024];
char oldicon[1024];
char tempicon[1024];
char key_pressed[10];

int volume;
char key[10];
gboolean running;
gboolean started;

gboolean main_win_vis, pl_win_vis, eq_win_vis, prev_vis_state[3];

int display_mode = COUNT_UP;	/*	should be only 0 || 1 || 2	*/
int track_name_pos = 0;
int play_label_pos = 30;

static void get_config(void);
static void run_cb(void *data);

static void xmms_cb(void *data); 
static void close_cb(void *data); 
static void do_config (void *data);
static void swap_icon (void *data);
static void change_label (void *data);
static void cancel_config (void *data);
static void save_config (void *data);
static void saveclose_config (void *data);

static void play_cb(void); 
static void pause_cb(void);  
static void eject_cb(void); 
static void prev_cb(void);  
static void next_cb(void);  
static void stop_cb(void);   

int get_volume(void);
void set_volume(void);

static void xmms_vis_state (void);
static void display_routine (void *data);
static void draw_display_time (int, int);
static void do_mouse_press_stuff(void *data, Window w, int x, int y, int b);
static void do_track_name_display(char*);
static void window_vis (gboolean, gboolean, gboolean);
static gboolean is_playing(void);

static void key_press(void *data, Window w, char *key);

static void in_cb(void *data, Window w);
static void out_cb(void *data, Window w);

static void
get_config(void)
{
	Epplet_load_config();
	if (Epplet_query_config("ICON") == NULL) {
		Epplet_add_config("ICON",EROOT "/epplet_data/E-xmms/xmms-icon8.xpm");
		sprintf(icon,"%s/epplet_data/E-xmms/xmms-icon8.xpm",EROOT);
		Epplet_save_config();
	} else {
		sprintf(icon,Epplet_query_config("ICON"));
	}
}

/* main routine for hiding/showing */
static void
run_cb(void *data)
{
	gboolean is_running = running;
	running = xmms_remote_is_running(xmms_session);
	if (is_running != running) {
		if (running) {
			Epplet_gadget_hide(xmms_b);   
			Epplet_gadget_hide(label);
			Epplet_gadget_hide(xmms_config);
			Epplet_gadget_hide(label2);
			Epplet_gadget_show(play_b);
			Epplet_gadget_show(play_label);
			Epplet_gadget_show(pause_b);
			Epplet_gadget_show(eject_b);
			Epplet_gadget_show(prev_b);
			Epplet_gadget_show(next_b);
			Epplet_gadget_show(stop_b);
			Epplet_gadget_show(vslider);
			Epplet_gadget_show(hbar);
			get_volume();
		} else {
			Epplet_gadget_hide(play_b);   
			Epplet_gadget_hide(play_label);   
			Epplet_gadget_hide(pause_b);   
			Epplet_gadget_hide(eject_b);   
			Epplet_gadget_hide(prev_b);   
			Epplet_gadget_hide(next_b);   
			Epplet_gadget_hide(stop_b);   
			Epplet_gadget_hide(vslider);   
			Epplet_gadget_hide(hbar);   
			Epplet_gadget_show(label);    
		} 
	}
	Epplet_timer(run_cb, NULL, 0.5, "TIMER1"); 
	data = NULL;
}

static void
xmms_cb(void *data)
{
	mp_pid = Epplet_spawn_command(xmmsprog);
	Epplet_gadget_show(label2);
	return;
	data = NULL;
}

static void
close_cb(void *data)
{
	Epplet_unremember();
	Esync();
	exit(0);
	data = NULL;
}

static void
do_config (void *data)
{
	int i;
	static char buf[NUMICONS][1024];
	int n_per_row = ceil(NUMICONS / floor(sqrt(NUMICONS)));
	int side = 45;
	config = Epplet_create_window_config(n_per_row*side,ceil(side*NUMICONS/n_per_row)+30,"E-xmms Config", saveclose_config,NULL, NULL,NULL, cancel_config,NULL);
	Esnprintf(oldicon, sizeof(oldicon), icon);

	// dx is 45
	// dy is 45
	// dimensions are 43x43
	
	int ix = 1; // x pixel location
	int iy = 2; // y pixel location
	int ic = ix; // column counter
	int jc = iy; // row counter
	for (i = 1; i <= NUMICONS; i++) {

		Esnprintf(buf[i-1], sizeof(buf[i-1]), EROOT "/epplet_data/E-xmms/xmms-icon%i.xpm",i);
		Epplet_gadget_show(Epplet_create_image_button(buf[i-1],ix,iy,43,43,swap_icon,&buf[i-1]));

		if (ic < n_per_row) {
			ix += 45;
			ic++;
		} else { // restart the column number (ix), increment the row
			ix = 1;
			ic = 1;
			iy += 45;
			jc++;
		}
	}

	Epplet_window_show(config);
	Epplet_window_pop_context();
	return;
}

static void
swap_icon (void *data)
{
	sprintf(icon,data);
	change_label(data);
}

static void
change_label (void *data)
{
	static char bufc[1024];
	Epplet_gadget_destroy(label);
	Esnprintf(bufc, sizeof(bufc), icon);
	label =  Epplet_create_image(2, 2, 43, 43, bufc);
	Epplet_gadget_show(label);
}

static void
cancel_config (void *data)
{
	Esnprintf(icon, sizeof(icon), oldicon);
	change_label(data);
	Epplet_window_hide(config);
}

static void
save_config (void *data)
{
	Epplet_modify_config("ICON",icon);
	change_label(data);
}

static void
saveclose_config (void *data)
{
	Epplet_modify_config("ICON",icon);
	Epplet_save_config();
	Epplet_gadget_data_changed(label);
	change_label(data);
	Epplet_window_destroy(config);
}


static void
play_cb(void)
{
	xmms_remote_play(xmms_session);
}

static void
pause_cb(void) 
{ 
        xmms_remote_pause(xmms_session); 
}

static void
eject_cb(void) 
{ 
        xmms_remote_eject(xmms_session); 
} 

static void
prev_cb(void)  
{  
        xmms_remote_playlist_prev(xmms_session);  
}

static void
next_cb(void)  
{  
        xmms_remote_playlist_next(xmms_session);  
}

static void
stop_cb(void)  
{  
        xmms_remote_stop(xmms_session);
	
}

int
get_volume(void)
{
	int vol;
	vol = xmms_remote_get_main_volume(xmms_session);
	volume = (100-vol)/5;
}

void
set_volume(void)
{
	int vol;
	vol = 100-(volume*5);
	xmms_remote_set_main_volume(xmms_session, vol);
}

static void
xmms_vis_state (void)
{

	main_win_vis = xmms_remote_is_main_win(xmms_session);
	pl_win_vis = xmms_remote_is_pl_win(xmms_session);
	eq_win_vis = xmms_remote_is_eq_win(xmms_session);

	if (main_win_vis) {
		prev_vis_state[0] = main_win_vis;
		prev_vis_state[1] = pl_win_vis;
		prev_vis_state[2] = eq_win_vis;
	}

	Epplet_timer((void*)xmms_vis_state, NULL, 0.5, "TIMER2");
}

static void
display_routine (void *data)
{

	/*	the following are:  current minute, current second of the current minute,	*
	 *	position of the current track in terms of seconds, and the current playlist	*
	 *	number	*/
	int m, s, temp;
	int list_position = xmms_remote_get_playlist_pos(xmms_session);

	/*	position of current track in thousandths of a second	*/
	pos = xmms_remote_get_output_time(xmms_session);
	if (pos < 1)
		pos = 1;

	/*	length of current track	*/
	len = xmms_remote_get_playlist_time(xmms_session, list_position);

	if (is_playing()) {
		if (len < 1) {
			len = 1;	
			newpos = 100;
		} else {
			newpos = (pos * 100) / len;
		}
	} else {
		newpos = 0;
	}

	if (display_mode == COUNT_UP) {
	
		temp = pos/1000;		/*	current track time in seconds	*/
		m = temp / 60;
		s = temp - (m * 60);
		draw_display_time(m, s);

	} else if (display_mode == COUNT_DOWN) {
	
		temp = (len - pos) / 1000;	/*	current track time counting down in seconds	*/
		m = temp / 60;
		s = temp - (m * 60);
		draw_display_time(m, s);

	}  else if (display_mode == TRACK) {
	
		do_track_name_display((char*)xmms_remote_get_playlist_title(xmms_session, list_position));

	}
	
	Epplet_gadget_data_changed(hbar);
	Epplet_gadget_data_changed(vslider);

	data = NULL;

}

static void
draw_display_time (int m, int s) //, char *pos2)
{

	/* decide when and where to put a zero into the	*
	 *	time string, for visual asthetics	*/
	 
 	char pos2[10];

	if (m < 10 && s < 10)
		sprintf(pos2, "0%li:0%li", m,s);
	else if (m < 10)
		sprintf(pos2, "0%li:%li",  m,s);
	else if (s < 10)
		sprintf(pos2, "%li:0%li",  m,s);
	else
		sprintf(pos2, "%li:%li",   m,s);

	Epplet_change_label(play_label, pos2);
	Epplet_timer(display_routine, NULL, 0.05, "TIMER");

}

static void
do_mouse_press_stuff (void *data, Window w, int x, int y, int b)
{

	int movepos;
	if (running && x>=11 && y<14) {

		/*	mouse button 1, toggle between couting up, counting down,	*
		 *	or displaying the track name	*/
		if (b==1) {
		
			if (display_mode == COUNT_UP && len == 1)
				display_mode = TRACK;
			else if (display_mode == COUNT_UP)
				display_mode = COUNT_DOWN;
			else if (display_mode == COUNT_DOWN)
				display_mode = TRACK;
			else if (display_mode == TRACK)
				display_mode = COUNT_UP;			
			else
				display_mode = COUNT_UP;

		/*	mouse button 3, toggles visibility of xmms main window	*
		 *	on my logitech wheel mouse, the middle is considered	*
		 *	second, apparently, and the right button third.	*/
		} else if (b==3) {

			if (main_win_vis)
				window_vis(FALSE, FALSE, FALSE);
			else
				window_vis(prev_vis_state[0], prev_vis_state[1], prev_vis_state[2]);

		}
	}
	if (running && x >= 12 && x <= 44 && y >= 15 && y <= 20 && len > 1 && b == 1) {
		movepos = ((float)x-12)/33*len;
		xmms_remote_jump_to_time(xmms_session, movepos);
	}
}

static void
do_track_name_display(char *track_name)
{
	/*	name_size is the array representing the text that is displayed on	*
	 *	the label widget.  name_size is the length of the string of the		*
	 *	track currently playing.  i is an index counter.	*/

	/* MAX_LEN is 9 */
	char name_sect[MAX_LEN];
	int name_size, i, j;

	name_size = strlen(track_name);

	if (name_size <= MAX_LEN) {
		Epplet_change_label (play_label, track_name);
	} else {
		for (i=0; i < MAX_LEN-1; i++) {
			if (isspace(track_name[track_name_pos+i]))
				name_sect[i] = ' ';
			else
				name_sect[i] = track_name[track_name_pos+i];
		}
		name_sect[i++] = '\0';
		counter++;
		Epplet_change_label (play_label, name_sect);
		if (track_name_pos < name_size && counter >= 0) {
			track_name_pos++;
		} else if (track_name_pos >= name_size) {
			track_name_pos = 0;
			counter=-10;
		}
	}
	free (track_name);

	Epplet_timer(display_routine, NULL, 0.15, "TIMER");

}

/*	Keeps track of the window visibility states.	*/
/*	So I don't have to use those 3 lines repeatedly.	*/
static void
window_vis (gboolean m, gboolean p, gboolean e)
{

	xmms_remote_main_win_toggle(xmms_session, m);
	xmms_remote_pl_win_toggle(xmms_session, p);
	xmms_remote_eq_win_toggle(xmms_session, e);

}

static gboolean
is_playing (void)
{
	return xmms_remote_is_playing(xmms_session);
}

/* now make this configurable
 */
static void
key_press(void *data, Window w, char *key)
{
	if (started) {
		if (*key == PREV)
			prev_cb();
		else if (*key == NEXT)
			next_cb();
		else if (*key == PLAY)
			play_cb();
		else if (*key == STOP)
			stop_cb();
		else if (*key == EJECT)
			eject_cb();
		else if (*key == PAUSE)
			pause_cb();
		else if (*key == VOL_UP) {
			if (volume > 0) {
				volume--;
				set_volume();
			}
		} else if (*key == VOL_DOWN) {
			if (volume < 20) {
				volume++;
				set_volume();
			}
		} else if (*key == TITLE) {
			if (display_mode == COUNT_UP && len == 1)
				display_mode = TRACK;
			else if (display_mode == COUNT_UP)
				display_mode = COUNT_DOWN;
			else if (display_mode == COUNT_DOWN)
				display_mode = TRACK;
			else if (display_mode == TRACK)
				display_mode = COUNT_UP;			
			else
				display_mode = COUNT_UP;
		}
	}
}

static void
in_cb(void *data, Window w)
{
	if (!running) {
		Epplet_gadget_show(xmms_b);
		Epplet_gadget_show(xmms_config);
	}
	Epplet_gadget_show(close_b);
	return;
}

static void
out_cb(void *data, Window w)
{
	if (!running) {
		Epplet_gadget_hide(xmms_b);
		Epplet_gadget_show(label);
		Epplet_gadget_hide(xmms_config);
	}
	Epplet_gadget_hide(close_b);
	return;
}


int main(int argc, char **argv)
{
	static char buf1[1024];
	static char buf2[1024];
	int prio;
   	started = FALSE;

	prio = getpriority(PRIO_PROCESS, getpid());
	setpriority(PRIO_PROCESS, getpid(), prio + 10);
	atexit(Epplet_cleanup);

	// the various timers
	Epplet_timer(display_routine, NULL, 0.05, "TIMER");
	Epplet_timer(run_cb, NULL, 0.5, "TIMER1");
	Epplet_timer((void*) xmms_vis_state, NULL, 0.5, "TIMER2");
	
	// init stuff
	Epplet_Init("E-xmms", "1.0", "E-xmms epplet", 3, 3, argc, argv, 0);
	get_config();

	// initial (xmms state = not running), and xmms loading icons
	Esnprintf(buf1, sizeof(buf1), "%s",icon);
	label =  Epplet_create_image(2, 2, 43, 43, buf1);
	Esnprintf(buf2, sizeof(buf1), EROOT "/epplet_data/E-xmms/xmms-init.xpm");
	label2 =  Epplet_create_image(2, 2, 43, 43, buf2);

	// buttons available in initial (xmms = not running) state
	xmms_config = Epplet_create_button(NULL, NULL, 2,  2, 0, 0, "CONFIGURE", 0, NULL, do_config, NULL);
	xmms_b      = Epplet_create_button(NULL, NULL, 22, 2, 0, 0, "SKIP",      0, NULL, xmms_cb,   NULL); 
	close_b     = Epplet_create_button(NULL, NULL, 34, 2, 0, 0, "CLOSE",     0, NULL, close_cb,  NULL);

	// buttons to control xmms - in order of appearance
	play_b  = Epplet_create_button(NULL, NULL, 10, 22, 0, 0, "PLAY",     0, NULL, (void*)play_cb,  NULL);
	pause_b = Epplet_create_button(NULL, NULL, 22, 22, 0, 0, "PAUSE",    0, NULL, (void*)pause_cb, NULL);
	eject_b = Epplet_create_button(NULL, NULL, 34, 22, 0, 0, "EJECT",    0, NULL, (void*)eject_cb, NULL);
	prev_b  = Epplet_create_button(NULL, NULL, 10, 34, 0, 0, "PREVIOUS", 0, NULL, (void*)prev_cb,  NULL);
	next_b  = Epplet_create_button(NULL, NULL, 22, 34, 0, 0, "NEXT",     0, NULL, (void*)next_cb,  NULL);
	stop_b  = Epplet_create_button(NULL, NULL, 34, 34, 0, 0, "STOP",     0, NULL, (void*)stop_cb,  NULL);

	// volume slider, and play label (time,title), and progress bar
	vslider = Epplet_create_vslider(2, 2, 44, 0, 20, 1, 5, &volume, (void*)set_volume, NULL);
	play_label = Epplet_create_label(11, 2, "", 1);
	hbar = Epplet_create_hbar(11, 14, 35, 8, 0, &newpos); // 100% if streaming

	// key bindings
	Epplet_register_key_press_handler(key_press, &key);

	// handler to deal with focus buttons
	Epplet_register_focus_in_handler(in_cb, NULL);
	Epplet_register_focus_out_handler(out_cb, NULL);
       
	Epplet_register_button_press_handler(do_mouse_press_stuff, NULL);

	Epplet_gadget_show(label);
	
	Epplet_show();
	// handle the state of E-xmms
	if (!started) {
		Epplet_remember();
		started = TRUE;
	}
	Epplet_Loop();

	return 0;
}
