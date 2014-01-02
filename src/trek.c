#include <pebble.h>
#include <stdlib.h>


static Window *window;
static TextLayer *text_layer;
static TextLayer* battery_text_layer;
static TextLayer* date_text_layer;
static TextLayer* temp_text_layer;
static GBitmap *insig_bitmap;
static BitmapLayer* insig_layer;
static GBitmap* bt_connected;
static BitmapLayer* bt_connected_layer;
static GFont trek30;
static GFont trek20;


static AppSync sync;
static uint8_t sync_buffer[64];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2,         // TUPLE_CSTRING
};


static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    /*
    case WEATHER_ICON_KEY:
      if (icon_bitmap) {
        gbitmap_destroy(icon_bitmap);
      }
      icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
      break;
    */
    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      //text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "temp = %s", new_tuple->value->cstring);
      text_layer_set_text(temp_text_layer,new_tuple->value->cstring);
      break;
    /*
    case WEATHER_CITY_KEY:
      text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
      */
  }
}


static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}


void handle_bt(bool connected){
  if(connected == 1){
  bt_connected = gbitmap_create_with_resource(RESOURCE_ID_bt_connected);
  bitmap_layer_set_bitmap(bt_connected_layer,bt_connected);
  }

  else{
  bt_connected = gbitmap_create_with_resource(RESOURCE_ID_bt_disconnected);
  bitmap_layer_set_bitmap(bt_connected_layer,bt_connected);
  }

}

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

    static char dateBuffer[] = "14 Aug";
    static char timeBuffer[] = "   00:00   ";

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

  trek30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_trekfont_30));
  trek20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_trekfont_20));


  /* Set up insig background */
  insig_bitmap = gbitmap_create_with_resource(RESOURCE_ID_starfleet_insignia);
  insig_layer = bitmap_layer_create(GRect(0,0,144,140));
  bitmap_layer_set_background_color(insig_layer,GColorBlack);
  bitmap_layer_set_bitmap(insig_layer,insig_bitmap);
  layer_add_child(window_layer,bitmap_layer_get_layer(insig_layer));


  
  
  /* set up text layer */
  text_layer = text_layer_create(GRect(0,138,144,50));
  text_layer_set_font(text_layer,trek30);
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  
  // set up date text layer
  date_text_layer = text_layer_create(GRect(120,0,40,40));
  text_layer_set_font(date_text_layer,trek20);
  text_layer_set_background_color(date_text_layer, GColorBlack);
  text_layer_set_text_color(date_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));
  

  //set up battery text layer
  battery_text_layer = text_layer_create(GRect(0,0,40,40));
  text_layer_set_font(battery_text_layer,trek20);
  text_layer_set_background_color(battery_text_layer, GColorBlack);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));

  //set up temp text layer
  temp_text_layer = text_layer_create(GRect(0,144,40,40));
  text_layer_set_font(temp_text_layer,trek20);
  text_layer_set_background_color(temp_text_layer, GColorBlack);
  text_layer_set_text_color(temp_text_layer, GColorWhite);
  text_layer_set_text(temp_text_layer, "-100'");
  layer_add_child(window_layer, text_layer_get_layer(temp_text_layer));


  bt_connected = gbitmap_create_with_resource(RESOURCE_ID_bt_disconnected);
  bt_connected_layer = bitmap_layer_create(GRect(115,135,40,40));
  bitmap_layer_set_background_color(bt_connected_layer,GColorBlack);
  layer_add_child(window_layer,bitmap_layer_get_layer(bt_connected_layer));

// create a time object to init time when window loaded
struct tm *t;
 time_t temp;    
 temp = time(NULL);  
 t = localtime(&temp);   
 tick_handler(t,MINUTE_UNIT);

//get battery charge when window loaded
 BatteryChargeState btchg = battery_state_service_peek();
 handle_battery(btchg);


 bool connected = bluetooth_connection_service_peek();
 handle_bt(connected);


 Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "1234"),
    TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();


}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);
  text_layer_destroy(text_layer);
  text_layer_destroy(battery_text_layer);
  text_layer_destroy(date_text_layer);
  text_layer_destroy(temp_text_layer);
  fonts_unload_custom_font(trek30);
  fonts_unload_custom_font(trek20);

}

static void init(void) {
  window = window_create();
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler); 
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bt);
  window_set_background_color(window,GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });


  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  gbitmap_destroy(insig_bitmap);
  gbitmap_destroy(bt_connected);
  bitmap_layer_destroy(insig_layer);
  bitmap_layer_destroy(bt_connected_layer);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
