#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_FT6206.h>

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 480;

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


  /*Create an table*/
  lv_obj_t *table = lv_table_create(lv_scr_act());
  /*Fill the first column*/
  lv_table_set_cell_value(table, 0, 0, "Name");
  lv_table_set_cell_value(table, 1, 0, "Apple");
  lv_table_set_cell_value(table, 2, 0, "Banana");
  lv_table_set_cell_value(table, 3, 0, "Lemon");
  lv_table_set_cell_value(table, 4, 0, "Grape");
  lv_table_set_cell_value(table, 5, 0, "Melon");
  lv_table_set_cell_value(table, 6, 0, "Peach");
  lv_table_set_cell_value(table, 7, 0, "Nuts");
  /*Fill the second column*/
  lv_table_set_cell_value(table, 0, 1, "Price");
  lv_table_set_cell_value(table, 1, 1, "$7");
  lv_table_set_cell_value(table, 2, 1, "$4");
  lv_table_set_cell_value(table, 3, 1, "$6");
  lv_table_set_cell_value(table, 4, 1, "$2");
  lv_table_set_cell_value(table, 5, 1, "$5");
  lv_table_set_cell_value(table, 6, 1, "$1");
  lv_table_set_cell_value(table, 7, 1, "$9");
  /*Set a smaller height to the table. It'll make it scrollable*/
  lv_obj_set_height(table, 200);
  lv_obj_center(table);
  /*Add an event callback to to apply some custom drawing*/
  lv_obj_add_event_cb(table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
}
static void draw_part_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    /*If the cells are drawn...*/
    if(dsc->part == LV_PART_ITEMS) {
        uint32_t row = dsc->id /  lv_table_get_col_cnt(obj);
        uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

        /*Make the texts in the first cell center aligned*/
        if(row == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
            dsc->rect_dsc->bg_color = lv_color_mix(lv_color_hex(0xE0878A), dsc->rect_dsc->bg_color, LV_OPA_20);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
        /*In the first column align the texts to the right*/
        else if(col == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
        }

        /*MAke every 2nd row grayish*/
        if((row != 0 && row % 2) == 0) {
            dsc->rect_dsc->bg_color = lv_color_mix(lv_color_hex(0xFFD4DF), dsc->rect_dsc->bg_color, LV_OPA_10);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
    }
}

void loop() {
  lv_timer_handler();
  delay(5);
}
