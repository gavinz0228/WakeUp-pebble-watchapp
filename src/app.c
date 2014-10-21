#include <pebble.h>
#include <math.h>
#include <stdlib.h>

  //144X168
void display_interval();
void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void battery_state_handler(BatteryChargeState state);
void update_time();
void update_date();
void set_random_mood();
void accel_tap_handler(AccelAxisType axis, int32_t direction);
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay);
void on_animation_stopped(Animation *anim, bool finished, void *context);
void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
void config_provider(Window* window);
char * int_to_string(int num);
Window *my_window;
TextLayer *text_layer;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *mood_layer; 
TextLayer *batbg_layer;
TextLayer *batfg_layer;
char interval_text[]="Break:";
int interval=30;
char *mood[]={":)",":(",":D",";)","8D"};
bool vibration_mode=false;
int second=30;
/*mood[0]=":)";
mood[1]=":(";
mood[2]=":D";
mood[3]";)";
mood[4]="8D";
*/
char *titleFont=FONT_KEY_GOTHIC_28_BOLD;
char *timeFont=FONT_KEY_DROID_SERIF_28_BOLD;
char *dateFont=FONT_KEY_GOTHIC_24;
char *moodFont=FONT_KEY_GOTHIC_24_BOLD;
int batteryHeight=3;
void handle_init(void) {
  my_window = window_create();
  window_set_background_color(my_window, GColorBlack);
  window_set_fullscreen(my_window, true);
  Layer *window_layer = window_get_root_layer(my_window);
  //-------text layer
  text_layer = text_layer_create(GRect(0, 0, 144, 40));
  text_layer_set_text_color(text_layer, GColorClear);
  text_layer_set_background_color(text_layer, GColorBlack);
  text_layer_set_font(text_layer,fonts_get_system_font(titleFont));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  //-------time layer
  time_layer = text_layer_create(GRect(0, 40, 144, 40));
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_background_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer,fonts_get_system_font(timeFont));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  //-------date layer
  date_layer = text_layer_create(GRect(0, 80, 144, 40));
  text_layer_set_text_color(date_layer, GColorClear);
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_font(date_layer,fonts_get_system_font(dateFont));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  //-------
  mood_layer = text_layer_create(GRect(0, 120, 144, 40));
  text_layer_set_text_color(mood_layer, GColorBlack);
  text_layer_set_background_color(mood_layer, GColorWhite);
  text_layer_set_font(mood_layer,fonts_get_system_font(moodFont));
  text_layer_set_text_alignment(mood_layer, GTextAlignmentCenter);
  //-------battery
  batbg_layer= text_layer_create(GRect(0, 168-batteryHeight, 144, batteryHeight));
  text_layer_set_background_color(batbg_layer, GColorWhite);
  batfg_layer= text_layer_create(GRect(0, 168-batteryHeight, 72, batteryHeight));
  text_layer_set_background_color(batfg_layer, GColorBlack);
  //-----
  
  
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(batbg_layer));
  layer_add_child(window_layer, text_layer_get_layer(batfg_layer));
  layer_add_child(window_layer, text_layer_get_layer(mood_layer));

  window_stack_push(my_window, true);
  display_interval();
  //-------substribe
  battery_state_service_subscribe(battery_state_handler);
  tick_timer_service_subscribe(SECOND_UNIT, minute_tick_handler);
  accel_tap_service_subscribe(accel_tap_handler);
  //minute_tick_handler(NULL, MINUTE_UNIT);
  battery_state_handler(battery_state_service_peek());
  update_time();
  update_date();
  set_random_mood();
  window_set_click_config_provider(my_window, (ClickConfigProvider)config_provider);
}
void config_provider(Window* window)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_single_click_handler);
}
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Window *window = (Window *)context;
  if(vibration_mode)
  {
    vibration_mode=false;
    text_layer_set_text(mood_layer, "Vibration: OFF");
  }
  else{
    vibration_mode=true;
    text_layer_set_text(mood_layer, "Vibration: ON");
  }
  
}
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(interval<120)
    interval++;
  display_interval();
}
void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(interval>2)
    interval--;
  display_interval();
}

void handle_deinit(void) {
  text_layer_destroy(text_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(batbg_layer);
  text_layer_destroy(batfg_layer);
  text_layer_destroy(mood_layer);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();

}
void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  if (units_changed&MINUTE_UNIT)
  {
    update_time();
  }
  else if(units_changed&DAY_UNIT)
    update_date();
  if(units_changed&SECOND_UNIT)
  {
    if(second>=interval)
    {
      if(vibration_mode)
        vibes_long_pulse();
      second=0;
    }
    else{
      second++;
    }
    display_interval();
    
  }
  
  
}

static void battery_state_handler(BatteryChargeState state)
{
  int len=(int)state.charge_percent*1.44;
  text_layer_set_size(batfg_layer, GSize(len, batteryHeight));
  
}
void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
  set_random_mood();
}

void update_time()
{  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char timeText[] = "00:00";
  if(clock_is_24h_style() == true) {
    strftime(timeText, sizeof(timeText), "%H:%M", tick_time);
  } else {
    strftime(timeText, sizeof(timeText), "%I:%M", tick_time);
  }

  GRect original =GRect(0, 40, 144, 40);
  GRect transition=GRect(0,40,144,0);
  animate_layer(text_layer_get_layer(time_layer), &original, &transition, 300, 500);
  text_layer_set_text(time_layer, timeText);
  animate_layer(text_layer_get_layer(time_layer), &transition, &original, 300, 500);
}

void update_date()
{
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char dateText[] = "Xxxxxxxxx 00";
  strftime(dateText, sizeof(dateText), "%B %e", tick_time);
  
  //GRect original =GRect(0, 40, 144, 40);
  //GRect transition=GRect(0,40,0,0);
  //animate_layer(text_layer_get_layer(date_layer), &original, &transition, 300, 500);
  text_layer_set_text(date_layer, dateText);
  //animate_layer(text_layer_get_layer(date_layer), &transition, &original, 300, 500);
}
void set_random_mood()
{
  int index=rand()%5;
  text_layer_set_text(mood_layer, mood[index]);
}
void display_interval()
{
  //siprintf(interv, "%d", interval);
  char * num_text=int_to_string(interval);
  /*
  char top_text[25];
  int i=0;
  while(i<(int)strlen(interval_text))
  {
    if(interval_text[i]=='\0')
      break;
    else
    {
       top_text[i]= interval_text[i];
    }
    i++;
  }
  int j=0;
  while(i<25)
  {
    if(num_text[j]=='\0'){
      top_text[i]=num_text[j];
      break;
    }
    else{
      top_text[i]=num_text[j];
    }
    i++;
    j++;
  }
  */
  text_layer_set_text(text_layer, num_text);
  free(num_text);
  //free(top_text);
}
char * int_to_string(int num)
{
  int string_size=13;
  char *temp_arr=(char*)malloc(string_size);
  temp_arr[string_size-1]='\0';
  char * pos=temp_arr+string_size-2;
  while(num!=0)
  {
      //cout<<(char)(48+num%10)<<endl;
      //cout<<"-"<<(int)(pos-iarr)<<endl;
    *pos=(char)(48+num%10);
    num=num/10;
    pos--;
  }
    pos++;
  char *arr=(char*)malloc(string_size);
  char *newpos;
  for (newpos=arr;pos<=temp_arr+string_size-1;pos++)
  {
    *newpos=*pos;
    newpos++;
  }
  *newpos='\0';
  free(temp_arr);
  return arr;
}
void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
    AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
    animation_schedule((Animation*) anim);
}

