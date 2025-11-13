#include "nds-pebble.h"

#include "pebble.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_num_buffer[6], s_day_buffer[6];
// Declare globally
static GFont s_time_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_batt_layer;
static GBitmap *s_batt_bitmap;
static int s_battery_level;
static Layer *s_battery_layer;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap_on;

static void bg_update_proc(Layer *layer, GContext *ctx) 
{
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, GColorWhite);
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
		const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
		const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
		gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
		gpath_draw_filled(ctx, s_tick_paths[i]);
	}
}

static void hands_update_proc(Layer *layer, GContext *ctx) 
{
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);
	center.y = center.y - 5;

	const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, (bounds.size.w / 2) - 34);
	const int16_t minute_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, (bounds.size.w / 2) - 40);
	const int16_t hour_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, (bounds.size.w / 2) - 46);

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	GPoint second_hand = {
		.x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + (center.y),
	};

	int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	GPoint minute_hand = {
		.x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + (center.y),
	};

	int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
	GPoint hour_hand = {
		.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.x,
		.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + (center.y),
	};

	// second hand
	graphics_context_set_stroke_width(ctx, 2);
	graphics_context_set_stroke_color(ctx, GColorDarkGray);
	graphics_draw_line(ctx, second_hand, center);

	// minute/hour hand
	//graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, minute_hand, center);

	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, hour_hand, center);

	//gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	//gpath_draw_filled(ctx, s_minute_arrow);
	//gpath_draw_outline(ctx, s_minute_arrow);

	//gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	//gpath_draw_filled(ctx, s_hour_arrow);
	//gpath_draw_outline(ctx, s_hour_arrow);

	// dot in the middle
	//graphics_context_set_fill_color(ctx, GColorBlack);
	//graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void date_update_proc(Layer *layer, GContext *ctx) 
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	strftime(s_day_buffer, sizeof(s_day_buffer), "%R", t);
	text_layer_set_text(s_day_label, s_day_buffer);

	strftime(s_num_buffer, sizeof(s_num_buffer), "%D", t);
	text_layer_set_text(s_num_label, s_num_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	layer_mark_dirty(window_get_root_layer(s_window));
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
	int width = (s_battery_level * 10) / 100;

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

static void window_load(Window *window) 
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NINTENDO_DS_BIOS_15));


	// Create GBitmap
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

	// Create BitmapLayer to display the GBitmap
	s_background_layer = bitmap_layer_create(bounds);

	// Set the bitmap onto the layer and add to the window
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));


	s_date_layer = layer_create(bounds);
	layer_set_update_proc(s_date_layer, date_update_proc);
	layer_add_child(window_layer, s_date_layer);

	s_day_label = text_layer_create(PBL_IF_ROUND_ELSE(
		GRect(50, -3, 29, 15),
		GRect(50, -3, 29, 15)));
	text_layer_set_text(s_day_label, s_day_buffer);
	text_layer_set_background_color(s_day_label, GColorWhite);
	text_layer_set_text_color(s_day_label, GColorBlack);
	text_layer_set_font(s_day_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

	s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
		GRect(83, -3, 27, 15),
		GRect(83, -3, 27, 15)));
	text_layer_set_text(s_num_label, s_num_buffer);
	text_layer_set_background_color(s_num_label, GColorWhite);
	text_layer_set_text_color(s_num_label, GColorBlack);
	text_layer_set_font(s_num_label, s_time_font);

	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

	s_hands_layer = layer_create(bounds);
	layer_set_update_proc(s_hands_layer, hands_update_proc);
	layer_add_child(window_layer, s_hands_layer);

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

static void window_unload(Window *window) 
{
	layer_destroy(s_date_layer);

	// Destroy BitmapLayer
	bitmap_layer_destroy(s_background_layer);

	// Destroy GBitmap
	gbitmap_destroy(s_background_bitmap);

	// Destroy BitmapLayer
	bitmap_layer_destroy(s_batt_layer);

	// Destroy GBitmap
	gbitmap_destroy(s_batt_bitmap);

	// Unload GFont
	fonts_unload_custom_font(s_time_font);

	text_layer_destroy(s_day_label);
	text_layer_destroy(s_num_label);

	layer_destroy(s_hands_layer);
	layer_destroy(s_battery_layer);

	gbitmap_destroy(s_bt_icon_bitmap);
	bitmap_layer_destroy(s_bt_icon_layer);
	gbitmap_destroy(s_bt_icon_bitmap_on);
}

static void init() 
{
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(s_window, true);

	s_day_buffer[0] = '\0';
	s_num_buffer[0] = '\0';

	// init hand paths
	s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

	Layer *window_layer = window_get_root_layer(s_window);
	GRect bounds = layer_get_bounds(window_layer);
	GPoint center = grect_center_point(&bounds);
	gpath_move_to(s_minute_arrow, center);
	gpath_move_to(s_hour_arrow, center);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
		s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}

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

static void deinit() 
{
	gpath_destroy(s_minute_arrow);
	gpath_destroy(s_hour_arrow);

	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
		gpath_destroy(s_tick_paths[i]);
	}

	tick_timer_service_unsubscribe();
	window_destroy(s_window);
}

int main() 
{
	init();
	app_event_loop();
	deinit();
}
