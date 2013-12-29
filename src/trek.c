#include <pebble.h>
#include <stdlib.h>


static Window *window;
static TextLayer *text_layer;
static TextLayer* battery_text_layer;
static TextLayer* date_text_layer;
static GBitmap *insig_bitmap;
static BitmapLayer* insig_layer;
char timeBuffer[] = "   00:00   ";
char dateBuffer[] = "14 Aug";







 void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
      snprintf(battery_text, sizeof(battery_text), "chg");
    } else {
        snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
      }
  text_layer_set_text(battery_text_layer, battery_text);
}




void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    //Here we will update the watchface display
    if(clock_is_24h_style()){
      strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", tick_time);
    }

    else{
    strftime(timeBuffer,sizeof(timeBuffer),"   %I:%M   ",tick_time);
    }
    strftime(dateBuffer, sizeof(dateBuffer), "%e %b", tick_time);
    text_layer_set_text(text_layer,timeBuffer);
    text_layer_set_text(date_text_layer, dateBuffer);
   
}





static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);


  /* Set up insig background */
  insig_bitmap = gbitmap_create_with_resource(RESOURCE_ID_starfleet_insignia);
  insig_layer = bitmap_layer_create(GRect(0,0,144,140));
  bitmap_layer_set_background_color(insig_layer,GColorBlack);
  bitmap_layer_set_bitmap(insig_layer,insig_bitmap);
  layer_add_child(window_layer,bitmap_layer_get_layer(insig_layer));
  
  
  /* set up text layer */
  text_layer = text_layer_create(GRect(0,138,144,50));
  text_layer_set_font(text_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_trekfont_30)));
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  
  // set up date text layer
  date_text_layer = text_layer_create(GRect(120,0,40,40));
  text_layer_set_font(date_text_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_trekfont_20)));
  text_layer_set_background_color(date_text_layer, GColorBlack);
  text_layer_set_text_color(date_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));
  

  //set up battery text layer
  battery_text_layer = text_layer_create(GRect(0,0,40,40));
  text_layer_set_font(battery_text_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_trekfont_20)));
  text_layer_set_background_color(battery_text_layer, GColorBlack);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));




// create a time object to init time when window loaded
struct tm *t;
 time_t temp;    
 temp = time(NULL);  
 t = localtime(&temp);   
 tick_handler(t,MINUTE_UNIT);

//get battery charge when window loaded
 BatteryChargeState btchg = battery_state_service_peek();
 handle_battery(btchg);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(battery_text_layer);
}

static void init(void) {
  window = window_create();
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler); 
  battery_state_service_subscribe(&handle_battery);
  window_set_background_color(window,GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  gbitmap_destroy(insig_bitmap);
  bitmap_layer_destroy(insig_layer);
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
