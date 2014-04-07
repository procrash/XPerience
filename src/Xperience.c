#include <pebble.h>

//#define true 1
// define false 0

const int SCREEN_WIDTH = 144;
const int SCREEN_HEIGHT = 144; // 168

//typedef int bool;

const int NR_FACES_DEFINED = 2;  // 2
const int NR_FACES_CALCULATED = 4;  // 2
const int NR_DIGITS_DEFINED = 6; // 6
const int NR_LINES_DEFINED = 4;

#define DIGIT_THICKNESS 6
#define SPACE_BETWEEN_DIGITS 1

// Settings (bit) flags
enum {
  SETTING_BLUETOOTH_SYMBOL     = 1 << 0,
  SETTING_BATTERY_SYMBOL       = 1 << 1,
  SETTING_CLOCK_STYLE          = 1 << 2,
  SETTING_DIGIT_SIZE           = 1 << 3,
  SETTING_SPACE_BETWEEN_DIGITS = 1 << 4
};

// Settings AppSync keys; correspond to appKeys in appinfo.json
enum {
  SETTING_SYNC_KEY_BLUETOOTH_SYMBOL     = 0,
  SETTING_SYNC_KEY_BATTERY_SYMBOL       = 1,
  SETTING_SYNC_KEY_CLOCK_STYLE          = 2,
  SETTING_SYNC_KEY_DIGIT_SIZE           = 3,
  SETTING_SYNC_KEY_SPACE_BETWEEN_DIGITS = 4
};


enum {
  CLOCK_STYLE_12_HOUR     =  0,
  CLOCK_STYLE_24_HOUR     =  1<<2
};
static uint8_t settings;



// Don't try to understand this! Took me a while...
static double clockFaces[2][6][4][2] =  // calculated Screen coordinates
{
	{ // Left
    {{65,68},{0,3},{0,3+DIGIT_THICKNESS},{65-DIGIT_THICKNESS,68}}, // TOP

    {{65-DIGIT_THICKNESS,75},{65,75},{31+DIGIT_THICKNESS/2,140-(31+DIGIT_THICKNESS/2)},{31,140-(31+DIGIT_THICKNESS)}},  // TOP Right, Mid Y is 104
                                    {{31-DIGIT_THICKNESS/2,140-31-DIGIT_THICKNESS/2},{31,140-31},{0,140},{0,140-DIGIT_THICKNESS}}, // Down Right
                                    
    {{DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS,77+SPACE_BETWEEN_DIGITS},{DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS,77+DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS},{31-SPACE_BETWEEN_DIGITS-DIGIT_THICKNESS/2,140-31-SPACE_BETWEEN_DIGITS-DIGIT_THICKNESS/2}, {31-SPACE_BETWEEN_DIGITS,140-(31+DIGIT_THICKNESS)-SPACE_BETWEEN_DIGITS}}, // Middle
    {{0,77}, {0+DIGIT_THICKNESS,77+DIGIT_THICKNESS},{0+DIGIT_THICKNESS,140-2*DIGIT_THICKNESS-SPACE_BETWEEN_DIGITS},{0,140-DIGIT_THICKNESS-SPACE_BETWEEN_DIGITS}}, // Down Left
    {{0,3+DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS}, {0+DIGIT_THICKNESS,3+2*DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS},{0+DIGIT_THICKNESS,70+DIGIT_THICKNESS},{0,70}} // Top Left
	},
	{ // Top
		{{3,0},{34-DIGIT_THICKNESS/2+3,34-DIGIT_THICKNESS/2},{34-DIGIT_THICKNESS/2+3+DIGIT_THICKNESS,34-DIGIT_THICKNESS/2},{3+DIGIT_THICKNESS,0}}, // Top Left
		{{3+DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS,0},{139-DIGIT_THICKNESS-SPACE_BETWEEN_DIGITS,0},{139-DIGIT_THICKNESS-SPACE_BETWEEN_DIGITS-DIGIT_THICKNESS,DIGIT_THICKNESS},{3+DIGIT_THICKNESS+SPACE_BETWEEN_DIGITS+DIGIT_THICKNESS,DIGIT_THICKNESS}}, // Top
		{{139-DIGIT_THICKNESS,0},{139,0},{139-(34-DIGIT_THICKNESS/2),34-DIGIT_THICKNESS/2},{139-(34-DIGIT_THICKNESS/2)-DIGIT_THICKNESS,34-DIGIT_THICKNESS/2}}, // Top Right
		{{34-DIGIT_THICKNESS/2+3+DIGIT_THICKNESS+ SPACE_BETWEEN_DIGITS ,34-DIGIT_THICKNESS/2},{139-(34-DIGIT_THICKNESS/2+SPACE_BETWEEN_DIGITS)-DIGIT_THICKNESS,34-DIGIT_THICKNESS/2},{139-(34+DIGIT_THICKNESS/2+SPACE_BETWEEN_DIGITS)-DIGIT_THICKNESS,34+DIGIT_THICKNESS/2},{34-DIGIT_THICKNESS/2+3+DIGIT_THICKNESS+ SPACE_BETWEEN_DIGITS+DIGIT_THICKNESS,34+DIGIT_THICKNESS/2}}, // Middle
		{{42,39},{42+DIGIT_THICKNESS,39},{68,65-DIGIT_THICKNESS},{68,65}}, // Bottom Left 
		{{102-DIGIT_THICKNESS,38},{102,38},{75,65},{75,65-DIGIT_THICKNESS}} // Bottom Right
	}
};



 int hourDigitsTens[3][6] = {
	{-1,  6,  7,  8, 10, 11}, // 0 10er digit
	{-1, -1, -1, -1,  8, 11}, // 1 10er digit 
	{-1, -1,  7,  8,  9, 10}  // 2 10er digit
};

 int hourDigitsOnes[10][6] =	{
	{-1, 18, 19, 20, 22, 23},
	{-1, -1, -1, -1, 20, 23}, // 1 ones digit
	{-1, -1, 18, 19, 21, 23},  // 2 ones digit
	{-1, -1, 19, 20, 21, 23}, // 3
	{-1, -1, -1, 20, 21, 23}, // 4
	{-1, -1, 19, 20, 21, 22}, // 5
	{-1, 18, 19, 20, 21, 22}, // 6
	{-1, -1, -1, 20, 22, 23}, // 7
	{18, 19, 20, 21, 22, 23}, // 8
	{-1, 29, 20, 21, 22, 23} // 9 
};

 int minuteDigitsTens[6][6] = {
	{-1,  0,  1,  2, 4, 5},
	{-1, -1, -1, -1, 4, 5},
	{-1, -1,  0,  1, 3, 4},
	{-1, -1,  0,  1, 2, 3},
	{-1, -1,  1,  2, 3, 5},
	{-1, -1,  0,  2, 3, 5},
};

 int minuteDigitsOnes[10][6] = {
	{-1, 12, 13, 14, 16, 17},
	{-1, -1, -1, -1, 16, 17},
	{-1, -1, 12, 13, 15, 16},
	{-1, -1, 12, 15, 16, 17},
	{-1, -1, -1, 15, 16, 17},
	{-1, -1, 12, 14, 15, 17},
	{-1, 12, 13, 14, 15, 17},
	{-1, -1, 13, 14, 16, 17},
	{12, 13, 14, 15, 16, 17},
	{-1, 12, 14, 15, 16, 17}
};

static int DIGIT_SIZE;
static int DIGIT_SPACING;

static bool USE_24H_CLOCK;
static bool selectedDigits[24];
					
// int clockFaceScreenCoordinates[4][6][4][2];

static GPathInfo facesDigits[4][6];
static GPath *calculatedFaceDigitPaths[4][6];
static GPoint facesDigitsPoints[4][6][5];
static int lastBatStatSummary = 0;

static AppSync settings_sync; /**< Keeps settings in sync between phone and watch */
static uint8_t settings_sync_buffer[62]; /**< Buffer used by settings sync */

static Layer *debugLayer;
static GContext *debugCtx;

static void determineScreenCoordinates();

static void updateDigits(int newDigitSize, int digitSpacing) {
	double newClockFaces[2][6][4][2] =  // calculated Screen coordinates
	{
		{ // Left
	    {{65,68},{0,3},{0,3+newDigitSize},{65-newDigitSize,68}}, // TOP

	    {{65-newDigitSize,75},{65,75},{31+newDigitSize/2,140-(31+newDigitSize/2)},{31,140-(31+newDigitSize)}},  // TOP Right, Mid Y is 104
	                                    {{31-newDigitSize/2,140-31-newDigitSize/2},{31,140-31},{0,140},{0,140-newDigitSize}}, // Down Right
                                    
	    {{newDigitSize+digitSpacing,77+digitSpacing},{newDigitSize+digitSpacing,77+newDigitSize+digitSpacing},{31-digitSpacing-newDigitSize/2,140-31-digitSpacing-newDigitSize/2}, {31-digitSpacing,140-(31+newDigitSize)-digitSpacing}}, // Middle
	    {{0,77}, {0+newDigitSize,77+newDigitSize},{0+newDigitSize,140-2*newDigitSize-digitSpacing},{0,140-newDigitSize-digitSpacing}}, // Down Left
	    {{0,3+newDigitSize+digitSpacing}, {0+newDigitSize,3+2*newDigitSize+digitSpacing},{0+newDigitSize,70+newDigitSize},{0,70}} // Top Left
		},
		{ // Top
			{{3,0},{34-newDigitSize/2+3,34-newDigitSize/2},{34-newDigitSize/2+3+newDigitSize,34-newDigitSize/2},{3+newDigitSize,0}}, // Top Left
			{{3+newDigitSize+digitSpacing,0},{139-newDigitSize-digitSpacing,0},{139-newDigitSize-digitSpacing-newDigitSize,newDigitSize},{3+newDigitSize+digitSpacing+newDigitSize,newDigitSize}}, // Top
			{{139-newDigitSize,0},{139,0},{139-(34-newDigitSize/2),34-newDigitSize/2},{139-(34-newDigitSize/2)-newDigitSize,34-newDigitSize/2}}, // Top Right
			{{34-newDigitSize/2+3+newDigitSize+ digitSpacing ,34-newDigitSize/2},{139-(34-newDigitSize/2+digitSpacing)-newDigitSize,34-newDigitSize/2},{139-(34+newDigitSize/2+digitSpacing)-newDigitSize,34+newDigitSize/2},{34-newDigitSize/2+3+newDigitSize+ digitSpacing+newDigitSize,34+newDigitSize/2}}, // Middle
			{{42,39},{42+newDigitSize,39},{68,65-newDigitSize},{68,65}}, // Bottom Left 
			{{102-newDigitSize,38},{102,38},{75,65},{75,65-newDigitSize}} // Bottom Right
		}
	};
	
	memcpy(clockFaces, newClockFaces, sizeof newClockFaces);
	
    determineScreenCoordinates();
    layer_mark_dirty(debugLayer);
}

static void determineScreenCoordinates() {
    // Find out minimum and maximum values of coordinate set
    double minX = 0;
    double maxX = 0;
    double minY = 0;
    double maxY = 0;
  
    bool minMaxInitialized = false;
  
    for (int screenFace = 0; screenFace < NR_FACES_DEFINED; screenFace++) {
  	  for (int digits = 0; digits < NR_DIGITS_DEFINED; digits++) {
  		  for (int lines = 0; lines< NR_LINES_DEFINED; lines++) {
  			  double x = clockFaces[screenFace][digits][lines][0];
  			  double y = clockFaces[screenFace][digits][lines][1];
  			  if (!minMaxInitialized) {
  				  minMaxInitialized = true;
  				  minX = x;
  				  maxX = x;
  				  minY = y;
  				  maxY = y;
  			  }
			  
			  if (x<minX) minX = x;
			  if (x>maxX) maxX = x;

			  if (y<minY) minY = y;
			  if (y>maxY) maxY = y;
  		  }
  	  }
    }


	maxX = 144;
	maxY = 144;
    // Convert coordinates into integers and adjust them to screen size and coordinate system...
    // Source Coordinate system is right and up positive
    // Dest Coordinate system is right and down positive
  
    // Flip first set horizontal and vertical,
    // Flip second horizontal only
    double xRangeCoordinateSource = maxX - minX;
    double yRangeCoordinateSource = maxY - minY;
  
    for (int screenFace = 0; screenFace < NR_FACES_DEFINED; screenFace++) {
  	  for (int digits = 0; digits < NR_DIGITS_DEFINED; digits++) {
		  facesDigits[screenFace][digits].num_points = 5;
		  facesDigits[screenFace+2][digits].num_points = 5;
				  
		  // GPoint pointsNormalFace[5];
		  // GPoint pointsFlippedFace[5];
		  
  		  for (int lines = 0; lines < NR_LINES_DEFINED; lines++) {
  			  double x = clockFaces[screenFace][digits][lines][0];
  			  double y = clockFaces[screenFace][digits][lines][1];
			  
  			  x-=minX;
  			  y-=minY;
			  
  			  int iX = x; // (int) ((x * (double)SCREEN_WIDTH) / xRangeCoordinateSource);
  			  int iY = y; //(int) ((y * (double)SCREEN_HEIGHT) / yRangeCoordinateSource);
			  
			  			  
			  facesDigitsPoints[screenFace][digits][lines].x = iX;
			  facesDigitsPoints[screenFace][digits][lines].y = iY;		
			  	  
			  
			  int iXFlipped = SCREEN_WIDTH-1 - iX;
			  int iYFlipped = SCREEN_HEIGHT-1 - iY;
			  			  
  			  if (screenFace == 0) {
				  facesDigitsPoints[screenFace+2][digits][lines].x = iXFlipped;
  		  	  } else
				  facesDigitsPoints[screenFace+2][digits][lines].x = iX;
			  
			  facesDigitsPoints[screenFace+2][digits][lines].y = iYFlipped;			  
			  
  		  }

		  facesDigitsPoints[screenFace][digits][NR_LINES_DEFINED].x = facesDigitsPoints[screenFace][digits][0].x;
		  facesDigitsPoints[screenFace][digits][NR_LINES_DEFINED].y = facesDigitsPoints[screenFace][digits][0].y;	
		  		  
		  facesDigitsPoints[screenFace+2][digits][NR_LINES_DEFINED].x = facesDigitsPoints[screenFace+2][digits][0].x;
		  facesDigitsPoints[screenFace+2][digits][NR_LINES_DEFINED].y = facesDigitsPoints[screenFace+2][digits][0].y;	  
		  
		  facesDigits[screenFace][digits].points = facesDigitsPoints[screenFace][digits];
		  facesDigits[screenFace+2][digits].points = facesDigitsPoints[screenFace+2][digits];
		  
		  calculatedFaceDigitPaths[screenFace][digits] = gpath_create(&facesDigits[screenFace][digits]);
		  calculatedFaceDigitPaths[screenFace+2][digits] = gpath_create(&facesDigits[screenFace+2][digits]);		  
		  
  	  }
    }
	
	
}


static Window *window;
static Layer *graphicslayer, *bluetooth_layer, *battery_layer;;


// static int digitSelected = 0;
static bool last_bluetooth;


static void draw_graphics(Layer *layer, GContext *ctx);



static void draw_graphics(Layer *layer, GContext *ctx) {
	
	
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	
	debugLayer = layer;
	debugCtx = ctx;
	
    graphics_context_set_stroke_color(ctx, GColorWhite);
	
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_color(ctx, GColorWhite);
		
	int count =0;
	
    for (int screenFace = 0; screenFace < NR_FACES_CALCULATED; screenFace++) {
  	  for (int digits = 0; digits < NR_DIGITS_DEFINED ; digits++) {
		  if (selectedDigits[count]) {
			  gpath_draw_outline(ctx, calculatedFaceDigitPaths[screenFace][digits]);
			  gpath_draw_filled(ctx, calculatedFaceDigitPaths[screenFace][digits]);
		  }
		  else {
			  gpath_draw_outline(ctx, calculatedFaceDigitPaths[screenFace][digits]);
		  }
		  
		  count++;
		  		  
	  }
    }

}

static void bluetooth_update_proc(Layer *layer, GContext *ctx) {

	// Clear layer...
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);	
		
	// Draw Layer if Bluetooth connection shall be displayed
	if ((settings & SETTING_BLUETOOTH_SYMBOL) == SETTING_BLUETOOTH_SYMBOL) {
  	  APP_LOG(APP_LOG_LEVEL_DEBUG, "BLUETOOTH_UPDATE");

  	  graphics_context_set_fill_color(ctx, GColorWhite);
  	  graphics_fill_rect(ctx, GRect(0, 0, 1, 7), 0, GCornerNone);
      graphics_fill_rect(ctx, GRect(21, 2, 1, 7), 0, GCornerNone);

  	  graphics_fill_rect(ctx, GRect(2, 2, 4, 2), 0, GCornerNone);
  	  graphics_fill_rect(ctx, GRect(16, 5, 4, 2), 0, GCornerNone);
  
  	  if(last_bluetooth) {
   	  	graphics_fill_rect(ctx, GRect(2, 4, 18, 1), 0, GCornerNone);
  	  } else {
    	graphics_fill_rect(ctx, GRect(2, 4, 8, 1), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(12, 4, 8, 1), 0, GCornerNone);

    	graphics_fill_rect(ctx, GRect(9, 3, 1, 1), 0, GCornerNone);
    	graphics_fill_rect(ctx, GRect(12, 5, 1, 1), 0, GCornerNone);
	  }
  	}
}

static void battery_update_proc(Layer *layer, GContext *ctx) {


	// Clear layer...
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);	
		
	// Draw Layer if Bluetooth connection shall be displayed
	if ((settings & SETTING_BATTERY_SYMBOL) == SETTING_BATTERY_SYMBOL) {
	  graphics_context_set_fill_color(ctx, GColorWhite);
	  graphics_fill_rect(ctx, GRect(0, 3, 1, 4), 0, GCornerNone);
	  graphics_context_set_stroke_color(ctx, GColorWhite);
	  graphics_draw_rect(ctx, GRect(1, 0, 21, 10));
  
	  if(lastBatStatSummary == 20) {
	    graphics_fill_rect(ctx, GRect(18, 2, 2, 6), 0, GCornerNone);
	  }
	  if(lastBatStatSummary == 30) {
	    graphics_fill_rect(ctx, GRect(18, 2, 2, 6), 0, GCornerNone);
	    graphics_fill_rect(ctx, GRect(12, 2, 5, 6), 0, GCornerNone);
	  }
	  if(lastBatStatSummary == 60) {
	    graphics_fill_rect(ctx, GRect(18, 2, 2, 6), 0, GCornerNone);
	    graphics_fill_rect(ctx, GRect(12, 2, 5, 6), 0, GCornerNone);
	    graphics_fill_rect(ctx, GRect(3, 2, 8, 6), 0, GCornerNone); 
	  }
	}
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  // Add graphics layer for watchface drawing...
  graphicslayer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(graphicslayer, draw_graphics);	 
  layer_add_child(window_layer, graphicslayer );

  // init bluetooth state layer
  bluetooth_layer = layer_create(GRect(4, 155, 22, 10));
  layer_set_update_proc(bluetooth_layer, bluetooth_update_proc);
  layer_add_child(window_layer, bluetooth_layer);

  // init battery state layer
  battery_layer = layer_create(GRect(118, 155, 22, 10));
  layer_set_update_proc(battery_layer, battery_update_proc);
  layer_add_child(window_layer, battery_layer);

  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "DONE");
  
}

static void window_unload(Window *window) {
  layer_destroy(graphicslayer);
  layer_destroy(bluetooth_layer);
  layer_destroy(battery_layer);
}


void timeChanged(struct tm *tick_time, TimeUnits units_changed) {

	 for (int i=0; i<24; i++) {
		 selectedDigits[i] = false;
	 }
	 
	 int hour = ((settings & SETTING_CLOCK_STYLE) == CLOCK_STYLE_24_HOUR) ? tick_time->tm_hour : tick_time->tm_hour % 12;
	 
	 int tensHour = hour / 10;
	 int onesHour = hour - tensHour*10;
	 
	 int tensMins = tick_time->tm_min / 10;
	 int onesMins = tick_time->tm_min - tensMins * 10;
	 
	 for (int i=0; i<6; i++) {
		 int sel = hourDigitsTens[tensHour][i];
		 if (sel!=-1) selectedDigits[sel] = true;
	 }
	 
	 for (int i=0; i<6; i++) {
		 int sel = hourDigitsOnes[onesHour][i];
		 if (sel!=-1) selectedDigits[sel] = true;
	 }
	 
	 for (int i=0; i<6; i++) {
		 int sel = minuteDigitsTens[tensMins][i];
		 if (sel!=-1) selectedDigits[sel] = true;
	 }

	 for (int i=0; i<6; i++) {
		 int sel = minuteDigitsOnes[onesMins][i];
		 if (sel!=-1) selectedDigits[sel] = true;
	 }
	 
	 layer_mark_dirty(debugLayer);
	 
}


static void handle_battery(BatteryChargeState charge_state) {
  
  int currentState = 0;
  int batteryState = charge_state.charge_percent;
  
  if(batteryState >= 20) {
    currentState = 20;
  }
  if(batteryState >= 30) {
    currentState = 30;
  }
  if(batteryState >= 60) {
    currentState = 60;
  }
  
  if (lastBatStatSummary!=currentState) {
    lastBatStatSummary = currentState;
    layer_mark_dirty(battery_layer); // Only mark layer dirty if we really have a need for a graphics change
  }
  
  
}

static void handle_bluetooth(bool connected) {
  last_bluetooth = connected;
  layer_mark_dirty(bluetooth_layer);
}


/**
 * Called when there is a settings sync error.
 *
 * @see https://developer.getpebble.com/2/api-reference/group___app_sync.html#ga144a1a8d8050f8f279b11cfb5d526212
 */
static void settings_sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Settings Sync Error: %d", app_message_error);
}


/**
 * Called when a settings tuple has changed.
 *
 * @todo only update if new_tuple != old_tuple?
 *
 * @see https://developer.getpebble.com/2/api-reference/group___app_sync.html#ga448af36883189f6345cc7d5cd8a3cc29
 */

static void settings_sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  uint8_t new = 0;

  switch (key) {
    case SETTING_SYNC_KEY_BLUETOOTH_SYMBOL:
      if (0 == ((uint8_t) new_tuple->value->uint8)) {
		  settings = settings & ~SETTING_BLUETOOTH_SYMBOL;
		  bluetooth_connection_service_unsubscribe();
	    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth Service unsubscribed");
	  }
      else {
		  settings = settings | SETTING_BLUETOOTH_SYMBOL;
		    battery_state_service_subscribe(&handle_battery);	
		    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth Service subscribed");
					
	  }
      break;
    case SETTING_SYNC_KEY_BATTERY_SYMBOL:
      if (0 == ((uint8_t) new_tuple->value->uint8)) {		  
		  settings = settings & ~SETTING_BATTERY_SYMBOL;
		  battery_state_service_unsubscribe();
  	    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Service unsubscribed");
	  }
      else {
		  settings = settings | SETTING_BATTERY_SYMBOL;
		  battery_state_service_subscribe(&handle_battery);
	    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery Service subscribed");
		  
	  }
	  
	
	  
      break;
    case SETTING_SYNC_KEY_CLOCK_STYLE:
	  if (0 == ((uint8_t) new_tuple->value->uint8)) {
	      APP_LOG(APP_LOG_LEVEL_DEBUG, "ClockStyle updated to 12 hour style");
		  settings = settings & ~SETTING_CLOCK_STYLE;
	  }
      else {
	      APP_LOG(APP_LOG_LEVEL_DEBUG, "ClockStyle updated to 24 hour style");
		  settings = settings | SETTING_CLOCK_STYLE;
		  
		  time_t now = time(NULL);
		  struct tm *t = localtime(&now);
		  timeChanged(t, MINUTE_UNIT);
		  
	  }
	  
      break;
    case SETTING_SYNC_KEY_DIGIT_SIZE:
      new = (uint8_t) new_tuple->value->uint8;
      if(new != DIGIT_SIZE) {
        DIGIT_SIZE = new;
	    updateDigits(DIGIT_SIZE, DIGIT_SPACING);
		
	    APP_LOG(APP_LOG_LEVEL_DEBUG, "Digit Size Updated: %d", DIGIT_SIZE);
      }
      break;
      case SETTING_SYNC_KEY_SPACE_BETWEEN_DIGITS:
        new = (uint8_t) new_tuple->value->uint8;
        if(new != DIGIT_SPACING) {
          DIGIT_SPACING = new;
		  
		  updateDigits(DIGIT_SIZE, DIGIT_SPACING);
		  
		  APP_LOG(APP_LOG_LEVEL_DEBUG, "Digit Spacing Updated: %d", DIGIT_SPACING);
        }
        break;

  }

  // Redraw watchface
  layer_mark_dirty(debugLayer);
/*
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
*/
}


static void init(void) {
	
  determineScreenCoordinates();
	
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_set_fullscreen(window, true);

  window_stack_push(window, animated);

  window_set_background_color(window, GColorBlack);
   
  handle_battery(battery_state_service_peek());
  last_bluetooth = false;
  handle_bluetooth(bluetooth_connection_service_peek());
  
  USE_24H_CLOCK = true; // clock_is_24h_style();
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  
  tick_timer_service_subscribe(MINUTE_UNIT, timeChanged);
  
//   layer_mark_dirty(battery_layer);
//     layer_mark_dirty(bluetooth_layer);

  // Load settings and init sync with JS app on phone
  Tuplet initial_settings[] = {
    TupletInteger(SETTING_SYNC_KEY_BLUETOOTH_SYMBOL, 0),
    TupletInteger(SETTING_SYNC_KEY_BATTERY_SYMBOL, 0),
    TupletInteger(SETTING_SYNC_KEY_CLOCK_STYLE, 0),
    TupletInteger(SETTING_SYNC_KEY_DIGIT_SIZE, 0),
	TupletInteger(SETTING_SYNC_KEY_SPACE_BETWEEN_DIGITS, 0)
  };
  app_sync_init(&settings_sync, settings_sync_buffer, sizeof(settings_sync_buffer), initial_settings, ARRAY_LENGTH(initial_settings),
    settings_sync_tuple_changed_callback, settings_sync_error_callback, NULL
  );
	
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Tuplet size: %ld", dict_calc_buffer_size_from_tuplets	(	initial_settings, 5));
	
	
  app_message_open(64, 64);


}

static void deinit(void) {
  window_destroy(window);

  for (int screenFace = 0; screenFace < NR_FACES_CALCULATED; screenFace++) {
	  for (int digits = 0; digits < NR_DIGITS_DEFINED; digits++) {
   	   	gpath_destroy(calculatedFaceDigitPaths[screenFace][digits]);
	}
  }
	  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

}

int main(void) {
  init();
  

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
