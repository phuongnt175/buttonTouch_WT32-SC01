#include <lvgl.h>
#include <lv_conf.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <FT6236.h>
#include <ui.h>

#define SDA_FT6236 18
#define SCL_FT6236 19
FT6236 ts = FT6236();

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

   ui_init();
}

void loop()
{
   lv_timer_handler(); /* let the GUI do its work */
   tft.setBrightness(brightnessValue);
   delay( 5 );
}