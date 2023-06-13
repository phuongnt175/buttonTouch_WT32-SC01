#include <lvgl.h>
#include <lv_conf.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <FT6236.h>
#include <ui.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define SDA_FT6236 18
#define SCL_FT6236 19
FT6236 ts = FT6236();

const char *ssid = "LUMI_TEST";
const char *password = "lumivn274!";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

int buttonState;
int ledState = LOW;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

extern lv_obj_t * ui_secondButton;
extern lv_obj_t * ui_firstButton;
extern lv_obj_t * ui_Screen1;
extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_Screen3;
extern lv_obj_t * ui_Screen4;
extern lv_obj_t * ui_Colorwheel1;
extern int brightnessValue;

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

/*Change to your screen resolution*/
static const uint32_t screenWidth  = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "On";
      }
      else { 
        inputChecked = false;
        outputStateM = "Off";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4> Living Room Light  </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"Living Room Light\" " "> <span class=\"slider\"></span></label>";
    buttons += "<h4> Kitchen Room Light </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"Kitchen Room Light\" " "><span class=\"slider\"></span></label>";
    buttons += "<h4> Curtain Mode       </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"Curtain Mode\" " ">      <span class=\"slider\"></span></label>";
    buttons += "<h4> AC                 </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"AC\" " ">                <span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

void setup()
{
  Serial.begin(115200);

  tft.begin();
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

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(10000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
  String inputMessage1;
  String inputMessage2;
  // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
    inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
    inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
    //digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    _ui_state_modify(ui_firstButton, LV_STATE_CHECKED, (inputMessage2.toInt()+1));
    ledState = !ledState;
  }
  else {
    inputMessage1 = "No message sent";
    inputMessage2 = "No message sent";
  }
  //Serial.print("GPIO: ");
  Serial.print(inputMessage1);
  Serial.print(" - : ");
  Serial.println(inputMessage2);
  request->send(200, "text/plain", "OK");
});

server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request)
{
  request->send(200, "text/plaint", String(lv_obj_get_state(ui_firstButton)).c_str());
});

  server.begin();

  ui_init();
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */

  int flag1 = lv_obj_get_state(ui_firstButton);

  //Serial.println(flag1);

  if (flag1 != buttonState) {
    buttonState = flag1;

    // only toggle the LED if the new button state is HIGH
    if (buttonState == HIGH) {
      ledState = !ledState;
    }
  }
  Serial.println(ledState);
  
  tft.setBrightness(brightnessValue); //change screen brightness according to slider's value

  //set screen bg color acording to color wheel selected
  lv_obj_set_style_bg_color(ui_Screen1, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen2, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen3, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen4, lv_colorwheel_get_rgb(ui_Colorwheel1), LV_PART_MAIN | LV_STATE_DEFAULT);

  delay( 5 );
}

// void ui_event_firstButton(lv_event_t * e)
// {
//   lv_event_code_t event_code = lv_event_get_code(e);
//   lv_obj_t * target = lv_event_get_target(e);
//   if(event_code == LV_EVENT_VALUE_CHANGED)
//   {

//   }
// }