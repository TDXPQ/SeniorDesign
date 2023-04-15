//============================================================================
//=======  OLED LCD DISPLAY MODULE EXECUTABLE INTERFACE CODE  ================
//=======                                                     ================
//=======  (Updated Fall 2022 RELEASE-2   ====================================


// OLED TEXT COLOR DEFINITIONS
// ***************************/
#define  BLACK          0x0000
#define  RED            0xF800
#define  INDIANRED      0xCAEB
#define  GOLDENROD      0xDD24
#define  YELLOW         0xFFE0
#define  ORANGE         0xFD20  // Current Night Color
#define  BLUE           0x001F
#define  MIDNIGHTBLUE   0x18CE
#define  DEEPSKYBLUE    0x05FF
#define  ORANGERED      0xFA20
#define  CRIMSON        0xD8A7
#define	 WHITE          0xFFFF



// Function PROTOTYPES
// =========================================================
void putcharOLED1(uint16_t c);
void putcharOLED2(uint16_t c);
void putcharOLED3(uint16_t c);
void oled_contrast_set_cc (uint8_t contrast);

void oled1_send_command(const uint16_t *array_ptr);
void oled2_send_command(const uint16_t *array_ptr);
void oled3_send_command(const uint16_t *array_ptr);

void oled1_putstring (const uint8_t *array_ptr);
void oled2_putstring (const uint8_t *array_ptr);
void oled3_putstring (const uint8_t *array_ptr);

void oled1_setxt_position(uint16_t xpos, uint16_t ypos);
void oled2_setxt_position(uint16_t xpos, uint16_t ypos);
void oled3_setxt_position(uint16_t xpos, uint16_t ypos);

void putcharOLED1(uint16_t c);
void putcharOLED2(uint16_t c);
void putcharOLED3(uint16_t c);



/***********************************************************************/
// CONSTANTS DEFINITIONS
//
// Command Table for fixed OLED-160-G2 Commands (TMS V1.0)
// Line format: <command word>, <command parameter words>
// ---------------------------------------------------------------------
//      ints        array name  {# ints/command, command, command params}
//     ------       -----------   --------       -----    --------
const uint16_t oled_clr_scrn[2] = {1,0xffD7}; // single word command
const uint16_t oled_drw_rect_outer[7]  = {6,0xFFCF,2,2,158,126,0xf800};
const uint16_t oled_drw_rect_inner[7]  = {6,0xFFCF,3,3,157,125,0xf800};
const uint16_t oled_drw_rect_inner2[7] = {6,0xFFCF,4,4,156,124,0xf800};
const uint16_t oled_drw_vline_mid[7] = {6,0xffD2, 80, 10, 80, 85, 0xf800};
const uint16_t oled_drw_hline_lo[7]      = {6,0xffD2, 15, 95, 144, 95, 0xf800};

const uint16_t oled_drw_line_bottom[7] = {6,0xffD2, 5, 123, 154, 123, 0xf800};
const uint16_t oled_drw_line_bottom2[7]  = {6,0xffD2, 5, 124, 154, 124, 0xf800};
const uint16_t oled_drw_line_bottom3[7] = {6,0xffD2, 5, 125, 154, 125, 0xf800};
const uint16_t oled_drw_line_lo[7]      = {6,0xffD2, 15, 64, 144, 64, 0xf800};
const uint16_t oled_drw_line_mid[7]     = {6,0xffD2, 7, 60, 152, 60, 0xf800};
const uint16_t oled_drw_line_hi[7]      = {6,0xffD2, 5, 56, 154, 56, 0xf800};
const uint16_t oled_drw_line_hi2[7]     = {6,0xffD2, 5, 48, 154, 48, 0xf800};
const uint16_t oled_drw_line_top[7]    = {6,0xffD2, 5, 3, 154, 3, 0xf800};
const uint16_t oled_drw_line_top2[7]   = {6,0xffD2, 5, 4, 154, 4, 0xf800};
const uint16_t oled_drw_line_top3[7]   = {6,0xffD2, 5, 5, 154, 5, 0xf800};

const uint16_t oled_drw_lf_otr_side[7]    = {6,0xffD2, 2, 2, 2, 126, 0xf800}; // Left outer line
const uint16_t oled_drw_rt_otr_side[7]    = {6,0xffD2, 2, 158, 2, 158, 0xf800}; // Left outer line

const uint16_t oled_setxt_height_talltall[3] = {2, 0xff7b,14};
const uint16_t oled_setxt_height_etall[3] = {2, 0xff7b,7};
const uint16_t oled_setxt_height_tall[3] = {2, 0xff7b,5};
const uint16_t oled_setxt_height[3] = {2, 0xff7b,4};
const uint16_t oled_setxt_height_med[3] = {2, 0xff7b,3};
const uint16_t oled_setxt_height_sml[3] = {2, 0xff7b,2};
const uint16_t oled_setxt_width_narrow[3] = {2, 0xff7c,1};
const uint16_t oled_setxt_width[3] = {2, 0xff7c,2};
const uint16_t oled_setxt_width_wide3[3] = {2, 0xff7c,3};
const uint16_t oled_setxt_width_wide4[3] = {2, 0xff7c,4};
const uint16_t oled_setxt_bold[3] = {2, 0xff76, 0x0001};   // BOLD TEXT


// TEXT COLORS DEFINED...
const uint16_t oled_setxt_FG_colorRED[3] = {2, 0xff7F, 0xf800};   // RED COLOR TEXT
const uint16_t oled_setxt_FG_colorORANGE[3] = {2, 0xff7F, 0xFD20};   // ORG COLOR TEXT
const uint16_t oled_setxt_FG_colorORANGERED[3] = {2, 0xff7F, 0xFA20};   // ORGRED COLOR



	
// const uint16_t oled_setxt_FG_colorWHEAT[3] = {2, 0xff7F, 0xF6F6};   // Off white color
// const uint16_t oled_setxt_FG_colorINDIANRED[3] = {2, 0xff7F, 0xCAEB};   // 
// const uint16_t oled_setxt_FG_colorCRIMSON[3] = {2, 0xff7F, 0xD8A7};   //    //BEST?
// const uint16_t oled_setxt_FG_colorHONEYDEW[3] = {2, 0xff7F, 0xF7FE};   // 
	
	
// const uint16_t oled_setxt_FG_colorHONEYDEW[3] = {2, 0xff7F, 0xFA20};   //  ORG-RED
const uint16_t oled_setxt_FG_colorORGRED[3] = {2, 0xff7F, 0xFA20};   //  ORG-RED





const uint16_t oled_setxt_FG_colorWHT[3] = {2, 0xff7F, 0xffff};   // WHITE COLOR TEXT

const uint16_t oled_baud_set[3] = {2, 0x000B, 0xcf};

//Static variable for decimal manipulation
static uint16_t dig_cnt;
static uint8_t sign;

