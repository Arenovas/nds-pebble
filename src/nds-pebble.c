#include <pebble.h>
#include "bgtest.h"

static Window *s_main_window;
static Layer *s_bgsq_layer;
static Layer *s_topbar_layer;
static Layer *s_clock_layer;
static GPath *s_tick_paths[NUM_CLOCK_TICKS];

static Layer *s_date_layer, *s_hands_layer;
static TextLayer *s_time_label, *s_date_label;
static char s_num_buffer[6], s_day_buffer[6];
// Declare globally
static GFont s_time_font;
static BitmapLayer *s_batt_layer;
static GBitmap *s_batt_bitmap;
static int s_battery_level;
static Layer *s_battery_layer;
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;
static GBitmap *s_bt_icon_bitmap_on;
static BitmapLayer *s_topsep_layer;
static GBitmap *s_topsep_bitmap;

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

	//Draw the second hand
	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
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

	// second hand
	graphics_context_set_stroke_width(ctx, 2);
	graphics_context_set_stroke_color(ctx, GColorRed);
	graphics_draw_line(ctx, second_hand, center);

	// minute/hour hand
	graphics_context_set_stroke_color(ctx, GColorDarkGray);
	graphics_draw_line(ctx, minute_hand, center);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, hour_hand, center);

	// dot in the middle
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 2, bounds.size.h / 2 - 2, 5, 5), 0, GCornerNone);
}

static void date_update_proc(Layer *layer, GContext *ctx)
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(s_day_buffer, sizeof(s_day_buffer), "%R", t);
	text_layer_set_text(s_time_label, s_day_buffer);

	strftime(s_num_buffer, sizeof(s_num_buffer), "%D", t);
	text_layer_set_text(s_date_label, s_num_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	layer_mark_dirty(window_get_root_layer(s_main_window));
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

	// Find the width of the bar
	int width = (s_battery_level * 9) / 100;

	// Draw the bar
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(2, 0, width, bounds.size.h), 0, GCornerNone);

	// Create GBitmap
	s_batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);

	// Create BitmapLayer to display the GBitmap
	s_batt_layer = bitmap_layer_create(bounds);

	bitmap_layer_set_compositing_mode(s_batt_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_batt_layer, s_batt_bitmap);
	layer_add_child(layer, bitmap_layer_get_layer(s_batt_layer));
}

static void bluetooth_callback(bool connected)
{
	// Show icon if disconnected
	if(connected)
	{
		bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
		bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap_on);
	}
	else
	{
		bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
		bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
	}
}

static void bgsq_proc(Layer *layer, GContext *ctx){
	GRect bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	#if PBL_DISPLAY_HEIGHT == 228
	//Draw Top Bar Line
	graphics_draw_line(ctx, GPoint(0, 0), GPoint(200, 0));
	for(int y = 0; y < 10; y++)
	{
		int posy = 1 + (24 * y);
		for(int x = 0; x < 10; x++)
		{
			int posx = -20 + (24 * x);
			graphics_draw_rect(ctx, GRect(posx, posy, 24, 24));
			if((x < 2 || x > 7) || (y < 1 || y > 6))
			{
				for(int lines = 0; lines < 8; lines++)
				{
					GPoint p1 = GPoint(3 + posx, 3 + (lines * 3) + posy);
					GPoint p2 = GPoint(20 + posx, 3 + (lines * 3) + posy);
					graphics_draw_line(ctx, p1, p2);
				}
			}
		}
	}
	//Draw Inner and Outer Clock Borders
	graphics_draw_rect(ctx, GRect(26, 23, 148, 148));
	graphics_draw_rect(ctx, GRect(25, 22, 150, 150));
	#elif PBL_DISPLAY_HEIGHT == 180
	for(int y = 0; y < 10; y++)
	{
		int posy = 0 + (16 * y);
		for(int x = 0; x < 12; x++)
		{
			int posx = -7 + (16 * x);
			graphics_draw_rect(ctx, GRect(posx, posy, 17, 17));
			if((x < 3 || x > 8) || (y < 1 || y > 6))
			{
				for(int lines = 0; lines < 8; lines++)
				{
					GPoint p1 = GPoint(2 + posx, 2 + (lines * 2) + posy);
					GPoint p2 = GPoint(14 + posx, 2 + (lines * 2) + posy);
					graphics_draw_line(ctx, p1, p2);
				}
			}
		}
	}
	//Draw Clock Border
	graphics_draw_rect(ctx, GRect(40, 15, 99, 99));
	#else
	for(int y = 0; y < 10; y++)
	{
		int posy = 0 + (16 * y);
		for(int x = 0; x < 10; x++)
		{
			int posx = -8 + (16 * x);
			graphics_draw_rect(ctx, GRect(posx, posy, 17, 17));
			if((x < 2 || x > 7) || (y < 1 || y > 6))
			{
				for(int lines = 0; lines < 8; lines++)
				{
					GPoint p1 = GPoint(2 + posx, 2 + (lines * 2) + posy);
					GPoint p2 = GPoint(14 + posx, 2 + (lines * 2) + posy);
					graphics_draw_line(ctx, p1, p2);
				}
			}
		}
	}
	//Draw Clock Border
	graphics_draw_rect(ctx, GRect(23, 15, 99, 99));
	#endif

}

static void topbar_proc(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);

	// Create GBitmap
	s_topsep_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TOPSEP_ICON);

	// Create BitmapLayer to display the GBitmap
	s_topsep_layer = bitmap_layer_create(GRect(0, 0, 1, 14));

	bitmap_layer_set_compositing_mode(s_topsep_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_topsep_layer, s_topsep_bitmap);
	layer_add_child(layer, bitmap_layer_get_layer(s_topsep_layer));
}

static void clock_proc(Layer *layer, GContext *ctx)
{
	//GRect bounds = layer_get_bounds(layer);
	//bounds.size.h -= 58;
	//bounds.size.w -= 49;
	//bounds.origin.y += 2;
	//bounds.origin.x += 25;
	graphics_context_set_fill_color(ctx, GColorBlack);
	/*//1
	graphics_fill_rect(ctx, GRect(70, 10, 4, 4), 0, GCornerNone);
	//2
	graphics_fill_rect(ctx, GRect(86, 26, 4, 4), 0, GCornerNone);
	//4
	graphics_fill_rect(ctx, GRect(86, 70, 4, 4), 0, GCornerNone);
	//5
	graphics_fill_rect(ctx, GRect(70, 86, 4, 4), 0, GCornerNone);
	//7
	graphics_fill_rect(ctx, GRect(26, 86, 4, 4), 0, GCornerNone);
	//8
	graphics_fill_rect(ctx, GRect(10, 70, 4, 4), 0, GCornerNone);
	//10
	graphics_fill_rect(ctx, GRect(10, 26, 4, 4), 0, GCornerNone);
	//11
	graphics_fill_rect(ctx, GRect(26, 10, 4, 4), 0, GCornerNone);
	//11
	graphics_fill_rect(ctx, GRect(47, 47, 5, 5), 0, GCornerNone);*/
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
	#elif PBL_DISPLAY_HEIGHT == 180
	int offset = 25;
	#else
	int offset = 15;
	#endif
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	GRect layer_bounds = bounds;
	layer_bounds.size.h -= offset;
	layer_bounds.origin.y += offset;
	s_bgsq_layer = layer_create(layer_bounds);

	layer_set_update_proc(s_bgsq_layer, bgsq_proc);
	layer_add_child(window_get_root_layer(window), s_bgsq_layer);

	//s_clock_layer = layer_create(GRect(23, 15, 99, 99));
	//layer_set_update_proc(s_clock_layer, clock_proc);
	//layer_add_child(s_bgsq_layer, s_clock_layer);

	#if PBL_DISPLAY_HEIGHT == 228
	s_clock_layer = layer_create(GRect(25, 44, 150, 150));
	#elif PBL_DISPLAY_HEIGHT == 180
	s_clock_layer = layer_create(GRect(40, 40, 99, 99));
	#else
	s_clock_layer = layer_create(GRect(23, 30, 99, 99));
	#endif
	layer_set_update_proc(s_clock_layer, clock_proc);
	layer_add_child(window_get_root_layer(window), s_clock_layer);

	layer_bounds.size.h = offset;
	layer_bounds.origin.y -= offset;
	s_topbar_layer = layer_create(layer_bounds);

	layer_set_update_proc(s_topbar_layer, topbar_proc);
	layer_add_child(window_get_root_layer(window), s_topbar_layer);

	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NINTENDO_DS_BIOS_15));

	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(s_topbar_layer, s_date_layer);

	s_time_label = text_layer_create(GRect(50, -3, 29, 15));
	text_layer_set_text(s_time_label, s_day_buffer);
	text_layer_set_text_color(s_time_label, GColorBlack);
	text_layer_set_font(s_time_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_time_label));

	s_date_label = text_layer_create(GRect(83, -3, 27, 15));
	text_layer_set_text(s_date_label, s_num_buffer);
	text_layer_set_text_color(s_date_label, GColorBlack);
	text_layer_set_font(s_date_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_date_label));
	GRect boundsClock = layer_get_bounds(s_clock_layer);

	s_hands_layer = layer_create(boundsClock);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(s_clock_layer, s_hands_layer);

	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(130, 4, 12, 7));
	layer_set_update_proc(s_battery_layer, battery_update_proc);

	// Add to Window
	layer_add_child(window_get_root_layer(window), s_battery_layer);

	// Create the Bluetooth icon GBitmap
	s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_OFF);
	s_bt_icon_bitmap_on = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON_ON);

	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_layer = bitmap_layer_create(GRect(114, 4, 11, 8));
	bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
}

static void main_window_unload(Window *window) {
	layer_destroy(s_bgsq_layer);
	layer_destroy(s_topbar_layer);
	layer_destroy(s_clock_layer);

	layer_destroy(s_date_layer);

	// Destroy BitmapLayer
	bitmap_layer_destroy(s_batt_layer);

	// Destroy GBitmap
	gbitmap_destroy(s_batt_bitmap);

	// Unload GFont
	fonts_unload_custom_font(s_time_font);

	// Destroy top bar text
	text_layer_destroy(s_time_label);
	text_layer_destroy(s_date_label);

	// Destroy watch hands
	layer_destroy(s_hands_layer);
	// Destroy battery icon
	layer_destroy(s_battery_layer);

	gbitmap_destroy(s_bt_icon_bitmap);
	bitmap_layer_destroy(s_bt_icon_layer);
	gbitmap_destroy(s_bt_icon_bitmap_on);
	bitmap_layer_destroy(s_topsep_layer);
	gbitmap_destroy(s_topsep_bitmap);
}

static void init() {
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
