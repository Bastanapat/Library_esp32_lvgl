#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_FT6206.h>

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 480;

lv_obj_t *list1;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
Adafruit_FT6206 ctp = Adafruit_FT6206();

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX = 0, touchY = 0;

  // bool touched = false;  //tft.getTouch( &touchX, &touchY, 600 );
  bool touched = ctp.touched();

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    // Get touch point
    TS_Point p = ctp.getPoint();
    // touchX = map(p.x, 0, 320, 320, 0);
    // touchY = map(p.y, 0, 480, 480, 0);
    touchX = p.x;
    touchY = p.y;
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(32, OUTPUT);
  digitalWrite(32, 1);
  // Setup capacitive touch
  if (!ctp.begin(40)) {
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1)
      ;
  }

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();
  tft.setRotation(2);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

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

  // Create a label
  lv_obj_t *ui_Label = lv_label_create(lv_scr_act());
  lv_obj_set_width(ui_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Label, 0);
  lv_obj_set_y(ui_Label, -100);
  lv_obj_set_align(ui_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Label, "TEST LVGL");
  lv_obj_set_style_text_color(ui_Label, lv_color_hex(0x32A852), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Create a LED List
  list1 = lv_list_create(lv_scr_act());
  lv_obj_set_size(list1, 180, 220);
  lv_obj_center(list1);

  lv_obj_t *btn;

  lv_list_add_text(list1, "File");
  btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
}
static void event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    String data = lv_list_get_btn_text(list1, obj);
    Serial.println(data);
  }
}


void loop() {
  lv_timer_handler();
  delay(5);
}
