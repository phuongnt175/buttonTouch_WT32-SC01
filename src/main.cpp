#include <lvgl.h>
#include <lv_conf.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <FT6236.h>
#include <ui.h>
#include <WiFi.h>
#include <WebServer.h>

#define SDA_FT6236 18
#define SCL_FT6236 19
FT6236 ts = FT6236();

const char *ssid = "LUMI_TEST";
const char *password = "lumivn274!";

WebServer server(80);

extern lv_obj_t * ui_secondButton;
extern lv_obj_t * ui_firstButton;
extern lv_obj_t * ui_Screen1;
extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_Screen3;
extern lv_obj_t * ui_Screen4;
extern lv_obj_t * ui_Colorwheel1;
class LGFX : public lgfx::LGFX_Device
{
lgfx::Panel_ST7796     _panel_instance;
lgfx::Bus_SPI       _bus_instance;   
lgfx::Light_PWM     _light_instance;

public:
  LGFX(void)
  {
    { 
      auto cfg = _bus_instance.config();    
      cfg.spi_host = VSPI_HOST;    
      cfg.spi_mode = 0;            
      cfg.freq_write = 40000000;   //40000000
      cfg.freq_read  = 16000000;    //16000000
      cfg.spi_3wire  = false;       
      cfg.use_lock   = true;        
      cfg.dma_channel = 1;         
      cfg.pin_sclk = 14;      //cau hinh chan SPI     
      cfg.pin_mosi = 13;          
      cfg.pin_miso = 12;          
      cfg.pin_dc   = 21;          
      _bus_instance.config(cfg);    
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();   
      cfg.pin_cs           =    15;  
      cfg.pin_rst          =    22;  
      cfg.pin_busy         =    -1;  
      cfg.memory_width     =   320;  
      cfg.memory_height    =   480;  
      cfg.panel_width      =   320;  
      cfg.panel_height     =   480;  
      cfg.offset_x         =     0; 
      cfg.offset_y         =     0;  
      cfg.offset_rotation  =     0; 
      cfg.dummy_read_pixel =     8;  
      cfg.dummy_read_bits  =     1;  
      cfg.readable         =  true;
      cfg.invert           = false;    
      cfg.rgb_order        = false;    
      cfg.dlen_16bit       = false;  
      cfg.bus_shared       =  true;  

      _panel_instance.config(cfg);
    }
    
    {  
      auto cfg = _light_instance.config();

      cfg.pin_bl = 23;
      cfg.invert = false;
      cfg.freq   = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

LGFX tft;

bool btn1Status = LOW;
bool btn2Status = LOW;
bool btn3Status = LOW;
bool btn4Status = LOW;

extern int brightnessValue;
static bool Flag1 = 0;
static bool Flag2 = 0;
static bool Flag3 = 0;
static bool Flag4 = 0;

/*Change to your screen resolution*/
static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

void handle_OnConnect();
void handle_btn1on();
void handle_btn2on();
void handle_btn3on();
void handle_btn4on();
void handle_btn1off();
void handle_btn2off();
void handle_btn3off();
void handle_btn4off();
void handle_NotFound();
String SendHTML(uint8_t btn1stat, uint8_t btn2stat, uint8_t btn3stat, uint8_t btn4stat);

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   //tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
   tft.endWrite();

   lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
   if(ts.touched()){
     data->state = LV_INDEV_STATE_PR;
     TS_Point p = ts.getPoint();
     data->point.x = p.y;
     data->point.y = tft.height() - p.x;
   }else{
     data->state = LV_INDEV_STATE_REL;
   }
}

void setup()
{
  Serial.begin(115200);

  tft.begin();
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Wifi connected...!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/btn1on", handle_btn1on);
  server.on("/btn2on", handle_btn2on);
  server.on("/btn3on", handle_btn3on);
  server.on("/btn4on", handle_btn4on);
  server.on("/btn1off", handle_btn1off);
  server.on("/btn2off", handle_btn2off);
  server.on("/btn3off", handle_btn3off);
  server.on("/btn4off", handle_btn4off);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  tft.setRotation(1);
  tft.setBrightness(255);

  if(!ts.begin(40, SDA_FT6236, SCL_FT6236)){
    Serial.println("Unable to start the capacitive touch Screen.");
  }

  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  server.handleClient();

  if(btn1Status != Flag1) {
    Flag1 = btn1Status;
    if(btn1Status)
    {
      _ui_state_modify(ui_firstButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_TOGGLE);
    }else{
      _ui_state_modify(ui_firstButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_REMOVE);
    }
  }

  if(btn2Status != Flag2) {
    Flag2 = btn2Status;
    if(btn2Status)
    {
      _ui_state_modify(ui_secondButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_TOGGLE);
    }else{
      _ui_state_modify(ui_secondButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_REMOVE);
    }
  }

  if(btn3Status != Flag3) {
    Flag3 = btn3Status;
    if(btn3Status)
    {
      _ui_state_modify(ui_thirdButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_TOGGLE);
    }else{
      _ui_state_modify(ui_thirdButton, LV_STATE_CHECKED,_UI_MODIFY_STATE_REMOVE);
    }
  }

  if(btn4Status != Flag4) {
    Flag4 = btn4Status;
    if(btn4Status)
    {
      _ui_state_modify(ui_forthButton, LV_STATE_CHECKED ,_UI_MODIFY_STATE_TOGGLE);
    }else{
      _ui_state_modify(ui_forthButton, LV_STATE_CHECKED ,_UI_MODIFY_STATE_REMOVE);
    }
  }

  tft.setBrightness(brightnessValue); //change screen brightness according to slider's value

  //set screen bg color acording to color wheel selected
  lv_obj_set_style_bg_color(ui_Screen1, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen2, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen3, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen4, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);

  delay( 5 );
}

void handle_OnConnect()
{
  btn1Status = LOW;
  btn2Status = LOW;
  btn3Status = LOW;
  btn4Status = LOW;
  Serial.println("Living room light off | Kitchen room light off");
  server.send(200, "text/html", SendHTML(btn1Status, btn2Status, btn3Status, btn4Status));
}

void handle_btn1on() {
  btn1Status = HIGH;
  Serial.println("Living room light on");
  server.send(200, "text/html", SendHTML(true, btn2Status, btn3Status, btn4Status));
}

void handle_btn2on() {
  btn2Status = HIGH;
  Serial.println("Kitchen room light on");
  server.send(200, "text/html", SendHTML(btn1Status, true, btn3Status, btn4Status));
}

void handle_btn3on() {
  btn3Status = HIGH;
  Serial.println("Curtain mode on");
  server.send(200, "text/html", SendHTML(btn1Status, btn2Status, true, btn4Status));
}

void handle_btn4on() {
  btn4Status = HIGH;
  Serial.println("AC on");
  server.send(200, "text/html", SendHTML(btn1Status, btn2Status, btn3Status, true));
}

void handle_btn1off() {
  btn1Status = LOW;
  Serial.println("Living room light off");
  server.send(200, "text/html", SendHTML(false, btn2Status, btn3Status, btn4Status));
}

void handle_btn2off() {
  btn2Status = LOW;
  Serial.println("Kitchen room light off");
  server.send(200, "text/html", SendHTML(btn1Status, false, btn3Status, btn4Status));
}

void handle_btn3off() {
  btn3Status = LOW;
  Serial.println("Curtain mode off");
  server.send(200, "text/html", SendHTML(btn1Status, btn2Status, false, btn4Status));
}

void handle_btn4off() {
  btn4Status = LOW;
  Serial.println("AC off");
  server.send(200, "text/html", SendHTML(btn1Status, btn2Status, btn3Status, false));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void ui_event_firstButton(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
      static uint8_t cnt = 0;
      cnt++;

      if(cnt % 2 ==0)
      {
        server.send(200, "text/html", SendHTML(false, btn2Status, btn3Status, btn4Status));
      }
      else{
        server.send(200, "text/html", SendHTML(true, btn2Status, btn3Status, btn4Status));
      }
      
    }
}

String SendHTML(uint8_t btn1stat, uint8_t btn2stat, uint8_t btn3stat, uint8_t btn4stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Station(STA) Mode</h3>\n";
  
  if(btn1stat)
  {ptr +="<p>Living Room light: ON</p><a class=\"button button-off\" href=\"/btn1off\">OFF</a>\n";}
  else
  {ptr +="<p>Living Room light: OFF</p><a class=\"button button-on\" href=\"/btn1on\">ON</a>\n";}

  if(btn2stat)
  {ptr +="<p>Kitchen Room light: ON</p><a class=\"button button-off\" href=\"/btn2off\">OFF</a>\n";}
  else
  {ptr +="<p>Kitchen Room light: OFF</p><a class=\"button button-on\" href=\"/btn2on\">ON</a>\n";}

  if(btn3stat)
  {ptr +="<p>Curtain mode: ON</p><a class=\"button button-off\" href=\"/btn3off\">OFF</a>\n";}
  else
  {ptr +="<p>Curtain mode: OFF</p><a class=\"button button-on\" href=\"/btn3on\">ON</a>\n";}

  if(btn4stat)
  {ptr +="<p>AC ON</p><a class=\"button button-off\" href=\"/btn4off\">OFF</a>\n";}
  else
  {ptr +="<p>AC OFF</p><a class=\"button button-on\" href=\"/btn4on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}