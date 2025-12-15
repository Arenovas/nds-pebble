#include <pebble.h>
#include "nds-pebble.h"

// Declare globally
static Window *s_main_window;

static Layer *s_bgsq_layer;
static Layer *s_topbar_layer;
static Layer *s_clock_layer;
static Layer *s_battery_layer;
static Layer *s_date_layer, *s_hands_layer;
static Layer *s_numbers_layer;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];

static GFont s_time_font;
static TextLayer *s_time_label, *s_date_label, *s_name_label;

static char s_num_buffer[6], s_day_buffer[6], s_name_buffer[11];
static int s_battery_level;

static BitmapLayer *s_batt_layer;
static BitmapLayer *s_bt_icon_on_layer;
static BitmapLayer *s_bt_icon_off_layer;
static BitmapLayer *s_topsep_layer;
static BitmapLayer *s_numbersg_layer;
static BitmapLayer *s_topbargradient_layer;

static GBitmap *s_batt_bitmap;
static GBitmap *s_bt_icon_bitmap_on;
static GBitmap *s_bt_icon_bitmap_off;
static GBitmap *s_topsep_bitmap;
static GBitmap *s_numbers_bitmap;
static GBitmap *s_topbargradient_bitmap;

//static const GColor bottomColors[] = { GColorMidnightGreen, GColorBulgarianRose};

// A struct for our specific settings (see main.h)
ClaySettings settings;

// Initialize the default settings
static void default_settings() {
	//settings.BackgroundColor = GColorBlack;
	//settings.ForegroundColor = GColorWhite;
	settings.SecondTick = true;
	settings.FavColor = 0;
	//settings.Animations = false;
}

// Read settings from persistent storage
static void load_settings() {
	// Load the default settings
	default_settings();
	// Read settings from persistent storage, if they exist
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	// Update the display based on new settings
	//update_display();
}

static void set_colors()
{
	bottomColors[0] = GColorMidnightGreen;
	bottomColors[1] = GColorBulgarianRose;
	bottomColors[2] = GColorRed;
	bottomColors[3] = GColorShockingPink;
	bottomColors[4] = GColorOrange;
	bottomColors[5] = GColorYellow;
	bottomColors[6] = GColorSpringBud;
	bottomColors[7] = GColorGreen;
	bottomColors[8] = GColorDarkGreen;
	bottomColors[9] = GColorJaegerGreen;
	bottomColors[10] = GColorVividCerulean;
	bottomColors[11] = GColorLiberty;
	bottomColors[12] = GColorDukeBlue;
	bottomColors[13] = GColorIndigo;
	bottomColors[14] = GColorImperialPurple;
	bottomColors[15] = GColorFashionMagenta;

	middleColors[0] = GColorCadetBlue;
	middleColors[1] = GColorWindsorTan;
	middleColors[2] = GColorSunsetOrange;
	middleColors[3] = GColorBabyBlueEyes;
	middleColors[4] = GColorChromeYellow;
	middleColors[5] = GColorIcterine;
	middleColors[6] = GColorInchworm;
	middleColors[7] = GColorSpringBud;
	middleColors[8] = GColorKellyGreen;
	middleColors[9] = GColorMediumAquamarine;
	middleColors[10] = GColorCyan;
	middleColors[11] = GColorBlueMoon;
	middleColors[12] = GColorBlue;
	middleColors[13] = GColorPurpureus;
	middleColors[14] = GColorPurple;
	middleColors[15] = GColorMagenta;

	topColors[0] = GColorCeleste;
	topColors[1] = GColorRajah;
	topColors[2] = GColorMelon;
	topColors[3] = GColorRichBrilliantLavender;
	topColors[4] = GColorIcterine;
	topColors[5] = GColorPastelYellow;
	topColors[6] = GColorPastelYellow;
	topColors[7] = GColorMintGreen;
	topColors[8] = GColorMayGreen;
	topColors[9] = GColorMintGreen;
	topColors[10] = GColorCeleste;
	topColors[11] = GColorVividCerulean;
	topColors[12] = GColorBlueMoon;
	topColors[13] = GColorLavenderIndigo;
	topColors[14] = GColorBrilliantRose;
	topColors[15] = GColorShockingPink;


}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	layer_mark_dirty(window_get_root_layer(s_main_window));
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	// Read boolean preferences
	Tuple *second_tick_t = dict_find(iter, MESSAGE_KEY_SecondTick);
	if(second_tick_t) {
		settings.SecondTick = second_tick_t->value->int32 == 1;
		tick_timer_service_unsubscribe();
		if(settings.SecondTick)
		{
			tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
		}
		else
		{
			tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
		}
	}
	Tuple *fav_color_t = dict_find(iter, MESSAGE_KEY_FavColor);
	if(fav_color_t)
	{
		settings.FavColor = atoi(fav_color_t->value->cstring);
	}
	save_settings();
}

static void hands_update_proc(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	center.y = center.y;
	const int16_t second_hand_length = (bounds.size.w / 2) - 12;
	const int16_t minute_hand_length = (bounds.size.w / 2) - 14;
	const int16_t hour_hand_length = (bounds.size.w / 2) - 23;

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int32_t second_angle;

	if (settings.SecondTick) {
		//Draw the second hand
		second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	}
	else
	{
		second_angle = 0;
	}
	GPoint second_hand = {
		.x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
	};

	//Draw the minute hand
	int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	GPoint minute_hand = {
		.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + center.y,
	};

	//Draw the hour hand
	int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
	GPoint hour_hand = {
		.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.y,
	};

	// minute/hour hand
	graphics_context_set_stroke_width(ctx, 2);
	graphics_context_set_stroke_color(ctx, GColorDarkGray);
	graphics_draw_line(ctx, minute_hand, center);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, hour_hand, center);

	if (settings.SecondTick) {
	// second hand
	//graphics_context_set_stroke_color(ctx, middleColors[settings.FavColor]);
	graphics_context_set_stroke_color(ctx, GColorRed);
	graphics_draw_line(ctx, second_hand, center);
	}

	// dot in the middle
	graphics_context_set_fill_color(ctx, GColorBlack);
	#if PBL_DISPLAY_HEIGHT == 228
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 4, bounds.size.h / 2 - 4, 8, 8), 0, GCornerNone);
	#else
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 2, bounds.size.h / 2 - 2, 5, 5), 0, GCornerNone);
	#endif
}

static void date_update_proc(Layer *layer, GContext *ctx)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	if(clock_is_24h_style())
	{
		strftime(s_day_buffer, sizeof(s_day_buffer), "%R", t);
		text_layer_set_text(s_time_label, s_day_buffer);
	}
	else
	{
		strftime(s_day_buffer, sizeof(s_day_buffer), "%r", t);
		text_layer_set_text(s_time_label, s_day_buffer);
	}
	strftime(s_num_buffer, sizeof(s_num_buffer), "%D", t);
	text_layer_set_text(s_date_label, s_num_buffer);
}

static void battery_callback(BatteryChargeState state)
{
	// Record the new battery level
	s_battery_level = state.charge_percent;
	// Update meter
	layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx)
{

	GRect bounds = layer_get_bounds(layer);
	#if PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorGreen);
	#else
		graphics_context_set_fill_color(ctx, GColorBlack);
	#endif

	#if PBL_DISPLAY_HEIGHT == 228
	// Find the width of the bar
	int width = (s_battery_level * 16) / 100;

	// Draw the bar
	graphics_fill_rect(ctx, GRect(4, 0, width, bounds.size.h), 0, GCornerNone);
	#else
	// Find the width of the bar
	int width = (s_battery_level * 8) / 100;

	// Draw the bar
	graphics_fill_rect(ctx, GRect(2, 0, width, bounds.size.h), 0, GCornerNone);
	#endif
}

static void bluetooth_callback(bool connected)
{
	// Show icon if disconnected
	if(connected)
	{
		layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_off_layer), true);
		layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_on_layer), false);
	}
	else
	{
		layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_off_layer), false);
		layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_on_layer), true);
	}
}

static void bgsq_proc(Layer *layer, GContext *ctx){
	GRect bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	#if PBL_DISPLAY_HEIGHT == 228
	graphics_draw_line(ctx, GPoint(0, 0), GPoint(200, 0));
	int yOffSet = 1;
	int xOffSet = -20;
	int ySqs = 10;
	int xSqs = 10;
	int sqSize = 24;
	int sqGridSize = 24;
	int lfLineX = 2;
	int rtLineX = 7;
	int lfLinePntOffSet = 3;
	int rtLinePntOffSet = 20;
	int lineVertOffSet = 3;
	#elif PBL_DISPLAY_HEIGHT == 180 || PBL_DISPLAY_HEIGHT == 168
	int yOffSet = 0;
	int ySqs = 10;
	int sqSize = 17;
	int sqGridSize = 16;
	int lfLinePntOffSet = 2;
	int rtLinePntOffSet = 14;
	int lineVertOffSet = 2;
	#endif

	#if PBL_DISPLAY_HEIGHT == 180
	int xOffSet = -7;
	int xSqs = 12;
	int lfLineX = 3;
	int rtLineX = 8;
	//Draw Clock Border
	//graphics_draw_rect(ctx, GRect(40, 15, 99, 99));
	#elif PBL_DISPLAY_HEIGHT == 168
	int xOffSet = -8;
	int xSqs = 10;
	int lfLineX = 2;
	int rtLineX = 7;
	//Draw Clock Border
	//graphics_draw_rect(ctx, GRect(23, 15, 99, 99));
	#endif

	#if PBL_COLOR
	graphics_context_set_stroke_color(ctx, GColorDarkGray);
	#else
	graphics_context_set_stroke_color(ctx, GColorBlack);
	#endif
	//Draw the background grid with top row first
	for(int y = 0; y < ySqs; y++)
	{
		//Get the Y position based on the offset from the top of the layer, grid size, and the Y level
		int posy = yOffSet + (sqGridSize * y);
		//Then calculate the X position of the grid square in the Y row
		for(int x = 0; x < xSqs; x++)
		{
			//Get the X position based on the offset from the left side of the layer, grid size, and the X level
			int posx = xOffSet + (sqGridSize * x);
			//Finally draw the grid square itself based on the previous positions and size
			//The sqGridSize should be 1 minus the sqSize but thanks to rects not using a stroke width of 2 correctly,
			//it has to be handled with a stroke width of 1 and thus has to be equal to sqSize
			graphics_context_set_stroke_color(ctx, GColorDarkGray);
			graphics_draw_rect(ctx, GRect(posx, posy, sqSize, sqSize));
			//Next, check if the current square is inside or outside of the current clock face bounds
			if((x < lfLineX || x > rtLineX) || (y < 1 || y > 6))
			{
				//If they aren't then iterate through and draw 8 lines with offsets from the left, right, and top of the current square
				for(int lines = 0; lines < 8; lines++)
				{
					graphics_context_set_stroke_color(ctx, GColorLightGray);
					GPoint p1 = GPoint(lfLinePntOffSet + posx, lineVertOffSet + (lines * lineVertOffSet) + posy);
					GPoint p2 = GPoint(rtLinePntOffSet + posx, lineVertOffSet + (lines * lineVertOffSet) + posy);
					graphics_draw_line(ctx, p1, p2);
				}
			}
		}
	}
	#if PBL_DISPLAY_HEIGHT == 228
	//Draw Inner and Outer Clock Borders
	//outer
	graphics_context_set_stroke_color(ctx, GColorLightGray);
	graphics_draw_rect(ctx, GRect(26, 23, 148, 148));
	//inner
	graphics_context_set_stroke_color(ctx, GColorLightGray);
	graphics_draw_rect(ctx, GRect(25, 22, 150, 150));
	//outer
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, GRect(28, 25, 144, 144));
	//inner
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, GRect(27, 24, 146, 146));
	#elif PBL_DISPLAY_HEIGHT == 180
	//Draw Clock Border
	graphics_context_set_stroke_color(ctx, GColorLightGray);
	graphics_draw_rect(ctx, GRect(40, 15, 99, 99));
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, GRect(41, 16, 97, 97));
	#elif PBL_DISPLAY_HEIGHT == 168

	#if PBL_COLOR
	//Draw Clock Border
	graphics_context_set_stroke_color(ctx, GColorLightGray);
	graphics_draw_rect(ctx, GRect(23, 15, 99, 99));
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, GRect(24, 16, 97, 97));
	#else
	//Draw Clock Border
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_rect(ctx, GRect(23, 15, 99, 99));
	#endif
	#endif
}

static void topbar_proc(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	#if PBL_COLOR
	//graphics_draw_bitmap_in_rect(ctx, s_topbargradient_bitmap, bounds);
	GColor* pal = gbitmap_get_palette(s_topbargradient_bitmap);
	//Bottom
	pal[0] = bottomColors[settings.FavColor];
	//Unused
	pal[1] = GColorRed;
	//Middle
	pal[2] = middleColors[settings.FavColor];
	//Top
	pal[3] = topColors[settings.FavColor];
	//gbitmap_set_palette(s_topbargradient_bitmap, pal, true);
	graphics_draw_bitmap_in_rect(ctx, s_topbargradient_bitmap, bounds);
	#else
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	#endif
}

static void clock_proc(Layer *layer, GContext *ctx)
{
	graphics_context_set_fill_color(ctx, GColorBlack);
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i)
	{
		const int x_offset = 0;
		const int y_offset = 0;
		gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
}

static void main_window_load(Window *window) {
	#if PBL_DISPLAY_HEIGHT == 228
	int offset = 22;
	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NINTENDO_DS_BIOS_20));
	s_time_label = text_layer_create(GRect(67, -3, 38, 22));
	s_date_label = text_layer_create(GRect(109, -3, 39, 22));
	s_name_label = text_layer_create(GRect(6, -3, 61, 22));
	s_topsep_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPSEPPT2_ICON);
	s_batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERYPT2_ICON);
	s_numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOCKNUMPT2_ICON);
	s_numbersg_layer = bitmap_layer_create(GRect(25, 44, 150, 150));
	s_clock_layer = layer_create(GRect(25, 44, 150, 150));
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(174, 4, 24, 14));
	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_on_layer = bitmap_layer_create(GRect(147, 1, 20, 20));
	s_bt_icon_off_layer = bitmap_layer_create(GRect(147, 1, 20, 20));
	// Create the Bluetooth icon GBitmap
	s_bt_icon_bitmap_off = gbitmap_create_with_resource(RESOURCE_ID_BTPT2_ICON_OFF);
	s_bt_icon_bitmap_on = gbitmap_create_with_resource(RESOURCE_ID_BTPT2_ICON_ON);
	s_topsep_layer = bitmap_layer_create(GRect(0, 0, 200, 22));
	s_topbargradient_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPBARPT2_ICON);
	#elif PBL_DISPLAY_HEIGHT == 180 || PBL_DISPLAY_HEIGHT == 168
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NINTENDO_DS_BIOS_15));
	s_topsep_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPSEP_ICON);
	s_batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);
	s_numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOCKNUM_ICON);
	#endif


	#if PBL_DISPLAY_HEIGHT == 180
	int offset = 25;
	s_time_label = text_layer_create(GRect(50, 8, 29, 15));
	s_date_label = text_layer_create(GRect(83, 8, 27, 15));
	s_name_label = text_layer_create(GRect(76, -4, 47, 15));
	s_numbersg_layer = bitmap_layer_create(GRect(40, 40, 99, 99));
	s_clock_layer = layer_create(GRect(40, 40, 99, 99));
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(130, 15, 12, 7));
	s_bt_icon_bitmap_off = gbitmap_create_with_resource(RESOURCE_ID_BTCOLOR_ICON_OFF);
	s_bt_icon_bitmap_on = gbitmap_create_with_resource(RESOURCE_ID_BTCOLOR_ICON_ON);
	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_on_layer = bitmap_layer_create(GRect(113, 10, 12, 13));
	s_bt_icon_off_layer = bitmap_layer_create(GRect(113, 10, 12, 13));
	s_topsep_layer = bitmap_layer_create(GRect(0, 11, 144, 15));
	s_topbargradient_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPBARRND_ICON);
	#elif PBL_DISPLAY_HEIGHT == 168
	int offset = 15;
	s_time_label = text_layer_create(GRect(50, -3, 29, 15));
	s_date_label = text_layer_create(GRect(83, -3, 27, 15));
	s_name_label = text_layer_create(GRect(3, -3, 47, 15));
	s_numbersg_layer = bitmap_layer_create(GRect(23, 30, 99, 99));
	s_clock_layer = layer_create(GRect(23, 30, 99, 99));
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(130, 4, 12, 7));
	// Create the BitmapLayer to display the GBitmap
	#if PBL_COLOR
	s_bt_icon_bitmap_off = gbitmap_create_with_resource(RESOURCE_ID_BTCOLOR_ICON_OFF);
	s_bt_icon_bitmap_on = gbitmap_create_with_resource(RESOURCE_ID_BTCOLOR_ICON_ON);
	s_bt_icon_on_layer = bitmap_layer_create(GRect(113, 1, 12, 13));
	s_bt_icon_off_layer = bitmap_layer_create(GRect(113, 1, 12, 13));
	#else
	s_bt_icon_bitmap_off = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_OFF);
	s_bt_icon_bitmap_on = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_ON);
	s_bt_icon_on_layer = bitmap_layer_create(GRect(114, 4, 11, 8));
	s_bt_icon_off_layer = bitmap_layer_create(GRect(114, 4, 11, 8));
	#endif

	s_topsep_layer = bitmap_layer_create(GRect(0, 0, 144, 15));
	s_topbargradient_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPBAR_ICON);
	#endif
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	GRect layer_bounds = bounds;
	layer_bounds.size.h -= offset;
	layer_bounds.origin.y += offset;
	s_bgsq_layer = layer_create(layer_bounds);
	s_numbers_layer = layer_create(layer_bounds);

	layer_set_update_proc(s_bgsq_layer, bgsq_proc);
	layer_add_child(window_get_root_layer(window), s_bgsq_layer);

	bitmap_layer_set_compositing_mode(s_numbersg_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_numbersg_layer, s_numbers_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_numbersg_layer));

	layer_set_update_proc(s_clock_layer, clock_proc);
	layer_add_child(window_get_root_layer(window), s_clock_layer);

	layer_bounds.size.h = offset;
	layer_bounds.origin.y -= offset;
	s_topbar_layer = layer_create(layer_bounds);

	layer_set_update_proc(s_topbar_layer, topbar_proc);
	layer_add_child(window_get_root_layer(window), s_topbar_layer);

	//s_topbargradient_layer = bitmap_layer_create(layer_bounds);

	//bitmap_layer_set_bitmap(s_topbargradient_layer, s_topbargradient_bitmap);
	//layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_topbargradient_layer));

	text_layer_set_background_color(s_time_label, GColorClear);
	text_layer_set_background_color(s_date_label, GColorClear);
	text_layer_set_background_color(s_name_label, GColorClear);
	s_date_layer = layer_create(bounds);
	#if PBL_COLOR
	if(settings.FavColor == 5 || settings.FavColor == 6 || settings.FavColor == 7)
	{
		text_layer_set_text_color(s_time_label, GColorBlack);
		text_layer_set_text_color(s_date_label, GColorBlack);
		text_layer_set_text_color(s_name_label, GColorBlack);
	}
	else
	{
		text_layer_set_text_color(s_time_label, GColorWhite);
		text_layer_set_text_color(s_date_label, GColorWhite);
		text_layer_set_text_color(s_name_label, GColorWhite);
	}
	#else
		text_layer_set_text_color(s_time_label, GColorBlack);
		text_layer_set_text_color(s_date_label, GColorBlack);
		text_layer_set_text_color(s_name_label, GColorBlack);
	#endif
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(s_topbar_layer, s_date_layer);

	text_layer_set_text(s_time_label, s_day_buffer);

	text_layer_set_font(s_time_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_time_label));

	text_layer_set_text(s_date_label, s_num_buffer);

	text_layer_set_font(s_date_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_date_label));

	strcpy(s_name_buffer, "Pebble");
	text_layer_set_text(s_name_label, s_name_buffer);

	text_layer_set_font(s_name_label, s_time_font);
	layer_add_child(s_date_layer, text_layer_get_layer(s_name_label));

	GRect boundsClock = layer_get_bounds(s_clock_layer);

	s_hands_layer = layer_create(boundsClock);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(s_clock_layer, s_hands_layer);

	bitmap_layer_set_compositing_mode(s_bt_icon_on_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bt_icon_on_layer, s_bt_icon_bitmap_on);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_on_layer));

	bitmap_layer_set_compositing_mode(s_bt_icon_on_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bt_icon_off_layer, s_bt_icon_bitmap_off);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_off_layer));

	layer_set_update_proc(s_battery_layer, battery_update_proc);
	layer_add_child(window_get_root_layer(window), s_battery_layer);

	s_batt_layer = bitmap_layer_create(layer_get_bounds(s_battery_layer));
	bitmap_layer_set_compositing_mode(s_batt_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_batt_layer, s_batt_bitmap);
	layer_add_child(s_battery_layer, bitmap_layer_get_layer(s_batt_layer));

	bitmap_layer_set_compositing_mode(s_topsep_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_topsep_layer, s_topsep_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_topsep_layer));
}

static void main_window_unload(Window *window) {
	layer_destroy(s_bgsq_layer);
	layer_destroy(s_topbar_layer);
	layer_destroy(s_clock_layer);

	layer_destroy(s_date_layer);
	layer_destroy(s_numbers_layer);

	// Destroy BitmapLayer
	bitmap_layer_destroy(s_batt_layer);

	// Destroy GBitmap
	gbitmap_destroy(s_batt_bitmap);

	// Unload GFont
	fonts_unload_custom_font(s_time_font);

	// Destroy top bar text
	text_layer_destroy(s_time_label);
	text_layer_destroy(s_date_label);
	text_layer_destroy(s_name_label);

	// Destroy watch hands
	layer_destroy(s_hands_layer);
	// Destroy battery icon
	layer_destroy(s_battery_layer);

	bitmap_layer_destroy(s_bt_icon_off_layer);
	gbitmap_destroy(s_bt_icon_bitmap_off);
	bitmap_layer_destroy(s_bt_icon_on_layer);
	gbitmap_destroy(s_bt_icon_bitmap_on);
	bitmap_layer_destroy(s_topsep_layer);
	gbitmap_destroy(s_topsep_bitmap);
	bitmap_layer_destroy(s_numbersg_layer);
	gbitmap_destroy(s_numbers_bitmap);
	bitmap_layer_destroy(s_topbargradient_layer);
	gbitmap_destroy(s_topbargradient_bitmap);
}

static void init() {
	set_colors();
	load_settings();
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);

	#if PBL_DISPLAY_HEIGHT == 228
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i)
	{
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS_EMERY[i]);
	}
	#else
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i)
	{
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}
	#endif

	s_day_buffer[0] = '\0';
	s_num_buffer[0] = '\0';

	tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	// Ensure battery level is displayed from the start
	battery_callback(battery_state_service_peek());

	// Register for Bluetooth connection updates
	connection_service_subscribe((ConnectionHandlers)
	{
		.pebble_app_connection_handler = bluetooth_callback
	});

	// Show the correct state of the BT connection from the start
	bluetooth_callback(connection_service_peek_pebble_app_connection());

	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(128, 128);
}

static void deinit() {
	// Destroy Window
	tick_timer_service_unsubscribe();
	window_destroy(s_main_window);
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i)
	{
		gpath_destroy(s_tick_paths[i]);
	}
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
