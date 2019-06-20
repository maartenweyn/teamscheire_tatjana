
/* ---------------- General defines ---------------- */
#define PART_MAIN_BG "bg"
#define PART_COLON_1 "colon1"
#define PART_COLON_2 "colon2"

#define STATE_DEFAULT "default"
#define STATE_CUSTOM "custom"
#define STATE_BTN_PRESSED "pressed"

#define FONT_STYLE_SMALL "BreezeSans:style=thin"
#define FONT_SIZE_SMALL 5*4
#define FONT_STYLE_THIN "BreezeSans:style=thin"
#define FONT_SIZE_THIN 12*4
#define FONT_STYLE_MEDIUM "BreezeSans:style=medium"
#define FONT_SIZE_MEDIUM 8*4
#define FONT_STYLE_BOLD "BreezeSans:style=bold"
#define FONT_SIZE_BOLD 14*4
#define FONT_SIZE_TITLE 5*4

/* ---------------- Setup defines ---------------- */
#define PART_BG "board_bg"
#define PART_START_BUTTON "start_button"
#define PART_INDICATOR_LEQ "indicator_leq"
#define PART_INDICATOR_LEQ_MIN "indicator_leq_min"
#define PART_INDICATOR_LEQ_HOUR "indicator_leq_hour"
#define PART_INDICATOR_LEQ_8HOUR "indicator_leq_8hour"
#define PART_INDICATOR_LEQ_DAY "indicator_leq_day"


#define IMAGE_FPATH_BOARD_BG "../res/images/board_bg.png"

#define IMAGE_FPATH_BUTTON_START "../res/images/button_start.png"
#define IMAGE_FPATH_BUTTON_START_PRESSED "../res/images/button_start_pressed.png"
#define IMAGE_FPATH_BUTTON_STOP "../res/images/button_stop.png"
#define IMAGE_FPATH_BUTTON_STOP_PRESSED "../res/images/button_stop_pressed.png"

#define IMAGE_FPATH_INDICATOR "../res/images/indicator.png"
#define IMAGE_FPATH_INDICATOR1 "../res/images/indicator1.png"
#define IMAGE_FPATH_INDICATOR8 "../res/images/indicator8.png"
#define IMAGE_FPATH_INDICATORD "../res/images/indicatorD.png"

#define DOT_ANGLE_START -120
#define DOT_ANGLE_END 120
#define PART_CLIPPER_REL_2_Y (260.0/360.0)

/* ---------------- Part macros ---------------- */
#define SOUND_PART(part_name, txt, rel_x, width, rel_y, height, f_size, f_name, color_r, color_g, color_b) \
	part {                                                                     \
		name: part_name;                                                       \
		type: TEXT;                                                            \
		description {                                                          \
			state: STATE_DEFAULT 0.0;                                          \
			rel1 {                                                             \
				relative: rel_x rel_y;                                         \
			}                                                                  \
			rel2 {                                                             \
				relative: rel_x+width rel_y+height;                            \
			}                                                                  \
			text {                                                             \
				text: txt;                                                     \
				size: f_size;                                                  \
				font: f_name;                                                  \
			}                                     \
			color: color_r color_g color_b 255;                                                           \
		}                                                                      \
		description {                                                          \
			state: STATE_PART_SELECTED 0.0;                                    \
			inherit:STATE_DEFAULT 0.0;                                         \
			text {                                                             \
				size: FONT_SIZE_BOLD;                                          \
				font: FONT_STYLE_BOLD;                                         \
			}                                                                  \
		}                                                                      \
	}                                                                          \

#define PART_INDICATOR(part_name, color_r, color_g, color_b, width, height) \
	part {                                                                  \
		name: part_name;                                                    \
		type: IMAGE;                                                        \
		description {                                                       \
			state: STATE_DEFAULT 0.0;                                       \
			rel1 {                                                          \
				relative: 0.5-(width/2.0/360.0) 0.0694;                     \
			}                                                               \
			rel2 {                                                          \
				relative: 0.5+(width/2.0/360.0) 0.0694+(height/360.0);      \
			}                                                               \
			image {                                                         \
				normal: IMAGE_FPATH_INDICATOR1;                       \
			}                                                               \
			color: color_r color_g color_b 255;                             \
			map {                                                           \
				on: 1;                                                      \
				smooth: 1;                                                  \
				perspective_on: 1;                                          \
				rotation {                                                  \
					center: PART_MAIN_BG;                                   \
					z: 0.0;                                                 \
				}                                                           \
			}                                                               \
		}                                                                   \
	}
