
//============================================================================
//           ===========================================
//===========  OLED LCD DISPLAY MODULE INTERFACE CODE   ======================
//           ===========================================
//=========== Updated Fall 2022 === RELEASE 2 VERSION =======================


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


//	RELOCATE?????????????????????????????????????????????????????????????????????????????????
//	RELOCATE?????????????????????????????????????????????????????????????????????????????????

//Static variable for decimal manipulation
static uint16_t dig_cnt;
static uint8_t sign;


/**********************************************************************
 OLED1 (Left) USART0   RECV CHAR ROUTINE
**********************************************************************
* uint16_t RxD_OLED1_USART0 (void);
*
* Description:
* -----------
*   Waits for: "RXCIF" to be set, and then returns...
*   Returns the "RXDATAL" byte/character...
**********************************************************************/
 uint16_t RxD_OLED1_USART0(void)
{
	uint8_t result8;

	while (!(USART0.STATUS & USART_RXCIF_bm)) {};
    result8 = USART0.RXDATAL;	
	return (int) (result8);
}

/**********************************************************************
 OLED1 (LEFT) USART0  SEND CHAR ROUTINE
**********************************************************************
* void TxD_OLED1_USART0 (unsigned uint8_t data);
*
* Description:
* -----------
*   Verifies transmit DATA register is empty, then loads "data" into
*   the data transmit register.
**********************************************************************/
void TxD_OLED1_USART0(uint8_t data )
{
	while (!(USART0.STATUS & USART_DREIF_bm))
	{
		;  // wait for DR to empty
	}
	USART0.TXDATAL = data;
}


/**********************************************************************
 OLED2 (Middle) USART3 RECV CHAR ROUTINE
**********************************************************************
* uint16_t RxD_OLED2_USART3 (void);
*
* Description:
* -----------
*   Waits for: "RXCIF" to be set, and then returns...
*   Returns the "RXDATAL" byte/character...
**********************************************************************/
uint16_t RxD_OLED2_USART3(void)
{
	uint8_t result8;

	while (!(USART3.STATUS & USART_RXCIF_bm)) {};
    result8 = USART3.RXDATAL;	
	
	return (int) (result8);
}


/**********************************************************************
 OLED2 (Middle) USART3   SEND CHAR ROUTINE
**********************************************************************
* void TxD_OLED2_USART3 (unsigned uint8_t data);
*
* Description:
* -----------
*   Verifies transmit DATA register is empty, then loads "data" into
*   the data transmit register.
**********************************************************************/

void TxD_OLED2_USART3(uint8_t data )
{
	while (!(USART3.STATUS & USART_DREIF_bm))
	{
		;  // wait for DR to empty
	}
	USART3.TXDATAL = data;
}



/**********************************************************************
 OLED3 (Right) USART1 RECV CHAR ROUTINE
**********************************************************************
* uint16_t RxD_OLED3_USART1 (void);
*
* Description:
* -----------
*   Waits for: "RXCIF" to be set, and then returns...
*   Returns the "RXDATAL" byte/character...
**********************************************************************/
uint16_t RxD_OLED3_USART1(void)
{
	uint8_t result8;

	while (!(USART1.STATUS & USART_RXCIF_bm)) {};
    result8 = USART1.RXDATAL;	
	
	return (int) (result8);
}


/**********************************************************************
 OLED3 (Right) USART1    SEND CHAR ROUTINE
**********************************************************************
* void TxD_OLED3_USART1 (unsigned uint8_t data);
*
* Description:
* -----------
*   Verifies transmit DATA register is empty, then loads "data" into
*   the data transmit register.
**********************************************************************/

void TxD_OLED3_USART1(uint8_t data )
{
	while (!(USART1.STATUS & USART_DREIF_bm))
	{
		;  // wait for DR to empty
	}
	USART1.TXDATAL = data;
}


/*********************************************************************
* OLED INIT - Resets and initializes all three OLED modules
*             (Timing Critial...)
*********************************************************************/
void oled_init (void)
{
	// OLED RESET: Reset ACTIVE Low (>2mS pulse)
//	PORTA.OUTCLR |= PIN2_bm;  //clear PA2 == OLED Reset == active LOW;
	PORTA.OUTCLR = PIN2_bm;  //clear PA2 == OLED Reset == active LOW;
	_delay_ms(15);

	// Take OLED Reset back to logic 1 == INACTIVE after at least >2mS pulse
	//PORTA.OUTSET |= PIN2_bm;  //set PA2 == OLED Reset == HIGH (inactive)
	PORTA.OUTSET = PIN2_bm;  //set PA2 == OLED Reset == HIGH (inactive)

	// CRITICAL DELAY!!!!!!!
	// OLED REQUIRES ~3 SECONDS BEFORE 1st COMMAND
	_delay_ms(2800);	// most recent == 2500 DID NOT WORK @ 8MHz
						//             == 2700 WORKED @ 8MHz

	// Clear the OLED busy flags...
	oled1_busy_flag = 0;  // When set to a '1', must wait executing cmd to end
	oled2_busy_flag = 0;  // 
	oled3_busy_flag = 0;  // 
			
	// Keep display off until loaded...
	
	// DEBUG SETTING :DEBUG VALUE == 3   Should be 1 for normal oper.
	contrast_level = 1;   // DEBUG VALUE == 3   Should be 1 for normal oper.
	contrast_oldval = contrast_level;
	oled_contrast_set_cc(contrast_level);  // Set to a very low level...
	oled_contrast_set_cc(contrast_level);  // Set to a very low level...
	return;
}


/**********************************************************************
***********************************************************************
* ALL PUTCHAR Routines   (OLED1, OLED2 & OLED3)
**********************************************************************
**********************************************************************/

/**********************************************************************
* void putcharOLED1(uint16_t c)
* Description:
* -----------
*   Uses the PUTCHAR OLED command to send a character to the OLED LCD
*
*   This routine replaces 'standard' putchar instruction
*********************************************************************/
void putcharOLED1(uint16_t c)  // USART0
{
	uint8_t send_byte;
	
	// Wait for any executing commands to end...
	while (oled1_busy_flag == 1) {   // Wait for RESPONSEs char to arrive

		if ((oled1_busy_flag == 1) && (USART0_STATUS & PIN7_bm)) {  // Char received??
			if ((USART0.RXDATAL) == 0x06) {
				oled1_busy_flag = 0;
				oled1_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled1_NAK_flag = 1;
				}
			}
	}
	
	//send bytes for PUTCHAR command (0xFFFE)
	send_byte = 0xFF;
	TxD_OLED1_USART0(send_byte);
	send_byte = 0xFE;
	TxD_OLED1_USART0(send_byte);
	
	//Send char as a word... MByte followed by LByte
	send_byte = 0x0;   // high word is 0
	TxD_OLED1_USART0(send_byte);
	send_byte = (char) (0x00ff & c);
	TxD_OLED1_USART0(send_byte);
	
	oled1_busy_flag = 1;
	return;
}


/**********************************************************************
* void putcharOLED2(uint16_t c)
* Description:
* -----------
*   Uses the PUTCHAR OLED command to send a character to the OLED LCD
*
*   This routine replaces 'standard' putchar instruction
*********************************************************************/
void putcharOLED2(uint16_t c)  // USART3
{
	uint8_t send_byte; 	
	
	// Wait for any executing commands to end...
	while (oled2_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) {  // Char received??
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
			}
			else { // NOT ACK, but NAK!
				oled2_NAK_flag = 1;
			}
		}
	}
		
	//send bytes for PUTCHAR command (0xFFFE)
	send_byte = 0xFF;
	TxD_OLED2_USART3(send_byte);
	send_byte = 0xFE;
	TxD_OLED2_USART3(send_byte);
	
	//Send char as a word... MByte followed by LByte
	send_byte = 0x0;   // high word is 0
	TxD_OLED2_USART3(send_byte);
	send_byte = (char) (0x00ff & c);
	TxD_OLED2_USART3(send_byte);
	
	oled2_busy_flag = 1;
	return;
}


/**********************************************************************
* void putcharOLED3(uint16_t c)
* Description:
* -----------
*   Uses the PUTCHAR OLED command to send a character to the OLED LCD
*
*   This routine replaces 'standard' putchar instruction
*********************************************************************/
void putcharOLED3(uint16_t c)  // USART3
{
	uint8_t send_byte;

	// Wait for any executing commands to end...
	while (oled3_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled3_busy_flag == 1) && (USART1_STATUS & PIN7_bm)) {  // Char received??
			if ((USART1.RXDATAL) == 0x06) {
				oled3_busy_flag = 0;
				oled3_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled3_NAK_flag = 1;
				}
			}
	}
	
	//send bytes for PUTCHAR command (0xFFFE)
	send_byte = 0xFF;
	TxD_OLED3_USART1(send_byte);
	send_byte = 0xFE;
	TxD_OLED3_USART1(send_byte);
	
	//Send char as a word... MByte followed by LByte
	send_byte = 0x0;   // high word is 0
	TxD_OLED3_USART1(send_byte);
	send_byte = (char) (0x00ff & c);
	TxD_OLED3_USART1(send_byte);
	
    oled3_busy_flag = 1;
	return;
}

// ------------------------------------------------------
// ------------------------------------------------------
// Contrast setting subroutines (For ALL THREE OLED Modules)
// Input = uint8_t = contrast level (0 to 15)
//
// Command format: [ cmd(MSB), cmd(LSB),   0, <constrast> ]
// ------------------------------------------------------
void oled_contrast_set_cc (uint8_t contrast)
{
 	uint8_t send_byte;

    // Wait for any executing commands to end...
	while ((oled1_busy_flag == 1) | (oled2_busy_flag == 1) | (oled3_busy_flag == 1)) {   // Wait for RESPONSEs char to arrive

		if ((oled1_busy_flag == 1) && (USART0_STATUS & PIN7_bm)) {  // Char received??
			if ((USART0.RXDATAL) == 0x06) {
				oled1_busy_flag = 0;
				oled1_NAK_flag = 0;
			   	}
			else { // NOT ACK, but NAK!
				oled1_NAK_flag = 1;
			   	}
			}

		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) {  // Char received??
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled2_NAK_flag = 1;
				}
			}

		if ((oled3_busy_flag == 1) && (USART1_STATUS & PIN7_bm)) {  // Char received??
			if ((USART1.RXDATAL) == 0x06) {
				oled3_busy_flag = 0;
				oled3_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled3_NAK_flag = 1;
				}
			}
		}
	
	// Snd 1st word = "set-contrast" cmd 
	// (0xFF66) to all OLEDs...
		
	// Send first cmd byte to each OLED...
	send_byte = 0xFF;
	TxD_OLED1_USART0(send_byte);
	TxD_OLED2_USART3(send_byte);
	TxD_OLED3_USART1(send_byte);

	// Send second cmd byte to each OLED...
	send_byte = 0x66;
	TxD_OLED1_USART0(send_byte);
	TxD_OLED2_USART3(send_byte);
	TxD_OLED3_USART1(send_byte);
		
	//send contrast_level word;
	send_byte = 0;
	TxD_OLED1_USART0(send_byte);
	TxD_OLED2_USART3(send_byte);
	TxD_OLED3_USART1(send_byte);
		
	send_byte = contrast;
	TxD_OLED1_USART0(send_byte);
	TxD_OLED2_USART3(send_byte);
	TxD_OLED3_USART1(send_byte);

	// Set all OLED busy flags ACTIVE...
	// (Reset by intr driven recv routines)
	oled1_busy_flag = 1;
	oled2_busy_flag = 1;
	oled3_busy_flag = 1;
	return;
}




// ------------------------------------------------------
// ------------------------------------------------------
// Contrast setting subroutines (For Middle OLED2/USART3 Module)
// Input = uint8_t = contrast level (0 to 15)
//
// Command format: [ cmd(MSB), cmd(LSB),   0, <constrast> ]
// ------------------------------------------------------
void oled2_contrast_set (uint8_t contrast)
{
 	uint8_t send_byte;

    // Wait for any executing commands to end...
	while ((oled2_busy_flag == 1)) {   // Wait for RESPONSEs char to arrive

		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) {  // Char received??
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled2_NAK_flag = 1;
				}
			}
		}
	
	// Snd 1st word = "set-contrast" cmd 
	// (0xFF66) to all OLEDs...
		
	// Send first cmd byte to each OLED...
	send_byte = 0xFF;
	TxD_OLED2_USART3(send_byte);

	// Send second cmd byte to each OLED...
	send_byte = 0x66;
	TxD_OLED2_USART3(send_byte);
		
	//send contrast_level word;
	send_byte = 0;
	TxD_OLED2_USART3(send_byte);
		
	send_byte = contrast;
	TxD_OLED2_USART3(send_byte);

	// Set all OLED busy flags ACTIVE...
	// (Reset by intr driven recv routines)
	oled2_busy_flag = 1;
	return;
}



/* ================================================================
 * OLED SEND COMMAND ROUTINES
 * ============================================================= */


/* ================================================================
* oled1_send_command (&pointer)   ]USART0]
*
* For the table driven "hard-coded", used for sending FIXED
* commands to uOLED1
*
* Inputs: *array_ptr (address to start of command string ints)
* --------------------------------------------------------------- */
void oled1_send_command (const uint16_t *array_ptr)
{
	uint16_t i, length, send_word; 
	uint8_t send_char_lo, send_char_hi; 
	const uint16_t *ptr;

	// Wait for any executing commands to end...
	while (oled1_busy_flag == 1) {   // Wait for RESPONSEs char to arrive

		if ((oled1_busy_flag == 1) && (USART0_STATUS & PIN7_bm)) {  // Char received??
			if ((USART0.RXDATAL) == 0x06) {
				oled1_busy_flag = 0;
				oled1_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled1_NAK_flag = 1;
				}
			}
	}
	
	//send command bytes, in while loop
	ptr = array_ptr;
	i = 0;
	length = *ptr++;   //load command length value
	while (i<length) //lenght == word count
		{
			send_word = *ptr++;
			send_char_hi = (char)((send_word & 0xff00) >> 8);
			send_char_lo = (char)(send_word & 0xff);

			//send high byte of word command
			TxD_OLED1_USART0 (send_char_hi);
			TxD_OLED1_USART0 (send_char_lo);
			i = i +1;
		}

	oled1_busy_flag = 1;
    return;
}



/* ================================================================
* oled2_send_command (&pointer)  [USART3]
*
* For the table driven "hard-coded", used for sending FIXED
* commands to uOLED1
*
* Inputs: *array_ptr (address to start of command string ints)
* --------------------------------------------------------------- */
void oled2_send_command (const uint16_t *array_ptr)
{
	uint16_t i, length, send_word;  
	uint8_t send_char_lo, send_char_hi; 
	const uint16_t *ptr;

	// Wait for any executing commands to end...
	while (oled2_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) {  // Char received??
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled2_busy_flag = 0;
				oled2_NAK_flag = 1;
				}
			}
	}
	
    // Send command in while loop
	ptr = array_ptr;
	i = 0;
	length = *ptr++;   //load command length value
	while (i<length) //lenght == word count
		{
			send_word = *ptr++;
			send_char_hi = (char)((send_word & 0xff00) >> 8);
			send_char_lo = (char)(send_word & 0xff);

			//send high byte of word command
			TxD_OLED2_USART3 (send_char_hi);
			TxD_OLED2_USART3 (send_char_lo);
			i = i +1;
		}

	oled2_busy_flag = 1;
	return;
}


/* ================================================================
* oled3_send_command (&pointer)   [USART1]
*
* For the table driven "hard-coded", used for sending FIXED
* commands to uOLED1
*
* Inputs: *array_ptr (address to start of command string ints)
* --------------------------------------------------------------- */
void oled3_send_command (const uint16_t *array_ptr)
{
	uint16_t i, length, send_word;    
	uint8_t send_char_lo, send_char_hi; 
	const uint16_t *ptr;
	
	// Wait for any executing commands to end...
	while (oled3_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled3_busy_flag == 1) && (USART1_STATUS & PIN7_bm)) {  // Char received??
			if ((USART1.RXDATAL) == 0x06) {
				oled3_busy_flag = 0;
				oled3_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled3_NAK_flag = 1;
				}
			}
	}
	

    // Send command in while loop
	ptr = array_ptr;
	i = 0;
	length = *ptr++;   //load command length value
	while (i<length) //lenght == word count
		{
			send_word = *ptr++;
			send_char_hi = (char)((send_word & 0xff00) >> 8);
			send_char_lo = (char)(send_word & 0xff);

			//send high byte of word command
			TxD_OLED3_USART1 (send_char_hi);
			TxD_OLED3_USART1 (send_char_lo);
			i = i +1;
		}
	
	oled3_busy_flag = 1;
	return;
}


// *********************************************************************
// Title/Plus "String Table" - For All Fixed Top Area Text Titles
// ---------------------------------------------------------------------
//   uint8_t     array name      {string}
//   ------      -----------     --------

const uint8_t Emergency[]  = "Emergency \0";
const uint8_t BrakeOn[]    = " BRAKE ON \0";
const uint8_t Shifter[]	  =  " Shifter \0";
const uint8_t In[]	      =     "   in    \0";
const uint8_t Neutral[]	  =     "  NEUTRAL \0";
const uint8_t Release[]	  =     " Release \0";
const uint8_t All[]	      =     "   All    \0";
const uint8_t Pedals[]	  =     "  PEDALS  \0";
const uint8_t Wait1[]	  =     "Wait!\0";       //WAIT message
const uint8_t DoNot[]     =     "  Do Not  \0";  //Power Up Screen
const uint8_t PRESS[]	  =     "  PRESS   \0";       //PRESS message
const uint8_t DontPress[] =     "Don't Press\0";  //Power Up Screen
const uint8_t Accel[]     =     "Accelerator\0";

const uint8_t Running[]    =     " Running  \0";
const uint8_t DIAGNOSTIC[] =     "Diagnostic\0";
const uint8_t Checks[]	=     " Checks   \0";
const uint8_t Soc_kWh[]   =     "SoC  kWh \0";  
const uint8_t ClrTextArea[]      =   "          \0"; 
const uint8_t ClrTextArea2[]     =   "           \0";
const uint8_t ClrTextAreaShort[] =   "       \0";  
const uint8_t ClrWait[]     =   "      \0";   
const uint8_t blnk_strng[] = "          \n";  // to clr temp dsp area
const uint8_t Fasten[]      =   "  FASTEN  \0";
const uint8_t SeatBelts[]  =  "Seat Belts\0";
const uint8_t TurnKey[] =  " Turn Key \0";
const uint8_t To[]      =  "    TO    \0";
const uint8_t For[]      =  "   FOR    \0";
const uint8_t Wait[]		=   "   Wait   \0";       //WAIT message
const uint8_t FiveSeconds[]	=   "5 Seconds \0";       //5 Seconds message
const uint8_t WaitFor[]	  =   " Wait For \0";       //WAIT message
const uint8_t Five[]		  =   "   FIVE   \0";       //FIVE message
const uint8_t ClrFive[]	  =   "          \0";       //FIVE message
const uint8_t Seconds[] =   "  Seconds \0";       //Seconds message
const uint8_t OnPosition[]  =  "ON Position\0";       //;       //
const uint8_t Powering[]    =  " Powering \0";
const uint8_t Up[]   =  "    Up    \0";
const uint8_t Vehicle [] = " Vehicle \0";

const uint8_t Motor[]   =    "  Motor  \0";
const uint8_t Control[] =    "  Control\0";
const uint8_t DCDC[]    =    "  DC-DC  \0";
const uint8_t BBox1[]   =    "  BBox-1 \0 ";  //NO SLASH ZERO!!!!!!!!!!!!!!!!
const uint8_t BBox2[]   =    "  BBox-2 \0";
const uint8_t Ambient[] =    " Ambient \0";
const uint8_t Battery[] =    " Battery \0";
const uint8_t High[]    =    "  HIGH \0";
const uint8_t Low[]     =    "   LOW \0";



/* ================================================================
 * OLED    P U T S T R I N G     COMMAND ROUTINES
 * ============================================================= */

/******************************************************************************
* oled1_putstring (const uint16_t Title_Num, const uint16_t *strng_ptr)
* [USART0]
******************************************************************************/
void oled1_putstring (const uint8_t *array_ptr)
{
	uint8_t send_char, send_byte;
	const uint8_t *ptr;

	// Entry...
	ptr = array_ptr;
	send_char = *ptr++;  // get first/next char of string

	// Wait for any executing commands to end...
	while (oled1_busy_flag == 1) {   //oled was busy?
		if ((oled1_busy_flag == 1) && (USART0_STATUS & PIN7_bm)) { // Chr rcvd??
			if ((USART0.RXDATAL) == 0x06) {
				oled1_busy_flag = 0;
				oled1_NAK_flag = 0;
			}
			else { // NOT ACK, but NAK!
				oled1_NAK_flag = 1;
					//	oled1_busy_flag = 0;
			}
		}
	}

	//send bytes for PUTSTRING command (0x0006)
	send_byte = 0x00;   //high byte
	TxD_OLED1_USART0(send_byte);
	send_byte = 0x06;  // low byte
	TxD_OLED1_USART0(send_byte);  // BYTE, not CHAR (below)

	// Send string chars...  NOT WORDS!
	while (send_char != 0)  {
		TxD_OLED1_USART0(send_char);
		send_char = *ptr++;  // get next/NL char from string
	}
	TxD_OLED1_USART0(send_char);  // send \0 char to end command
	oled1_busy_flag = 1;
	return;
}


/******************************************************************************
* oled2_putstring (const uint16_t Title_Num, const uint16_t *strng_ptr)
* [USART3]
******************************************************************************/
void oled2_putstring (const uint8_t *array_ptr)
{
	uint8_t send_char, send_byte; 
	const uint8_t *ptr;

	// Entry...
	ptr = array_ptr;
	send_char = *ptr++;  // get first/next char of string

	// Wait for any executing commands to end...
	while (oled2_busy_flag == 1) {   //oled was busy?
		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) { // Chr rec'd?
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled2_NAK_flag = 1;
				}
		}
	}

	//send bytes for PUTSTRING command (0x0006)
	send_byte = 0x00;   //high byte
	TxD_OLED2_USART3(send_byte);
	send_byte = 0x06;  // low byte
	TxD_OLED2_USART3(send_byte);  // BYTE, not CHAR (below)

	// Send string chars...  NOT WORDS!
	while (send_char != 0)  {
		TxD_OLED2_USART3(send_char);
		send_char = *ptr++;  // get next/NL char from string
	}
	TxD_OLED2_USART3(send_char);  // send \0 char to end command
	oled2_busy_flag = 1;
	return;
}



/******************************************************************************
* oled3_putstring (const uint16_t Title_Num, const uint16_t *strng_ptr)
* [USART1]
******************************************************************************/
void oled3_putstring (const uint8_t *array_ptr)
{
	uint8_t send_char, send_byte;
	const uint8_t *ptr;

	// Entry...
	ptr = array_ptr;
	send_char = *ptr++;  // get first/next char of string

	// Wait for any executing commands to end...
	while (oled3_busy_flag == 1) {   //oled was busy?
		if ((oled3_busy_flag == 1) && (USART1_STATUS & PIN7_bm)) { // Chr rcvd?
			if ((USART1.RXDATAL) == 0x06) {
				oled3_busy_flag = 0;
				oled3_NAK_flag = 0;
			}
			else { // NOT ACK, but NAK!
				oled3_NAK_flag = 1;
				//	oled3_busy_flag = 0;
			}
		}
	}

	//send bytes for PUTSTRING command (0x0006)
	send_byte = 0x00;   //high byte
	TxD_OLED3_USART1(send_byte);
	send_byte = 0x06;  // low byte
	TxD_OLED3_USART1(send_byte);  // BYTE, not CHAR (below)

	// Send string chars...  NOT WORDS!
	while (send_char != 0)  {
		TxD_OLED3_USART1(send_char);
		send_char = *ptr++;  // get next/NL char from string
	}
	TxD_OLED3_USART1(send_char);  // send \0 char to end command
	oled3_busy_flag = 1;
	return;
}



/* ================================================================
 * ================================================================
 * OLED   S E T  T E X T  P O S I T I O N    ROUTINES
 * ================================================================
 * ============================================================= */


/***********************************************************************
* oled1_setxt_position (Uses MOVE ORIGIN serial command)
*
* Inputs: y-pos int, x-pos int
*
***********************************************************************/
void oled1_setxt_position (const uint16_t xpos, const uint16_t ypos)
{
	uint16_t send_word;
	uint8_t send_char_lo, send_char_hi;

	// Wait for any executing commands to end...
	while (oled1_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled1_busy_flag == 1) && (USART0_STATUS & PIN7_bm)) {  
			if ((USART0.RXDATAL) == 0x06) {
				oled1_busy_flag = 0;
				oled1_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled1_NAK_flag = 1;
			}
		}
	}


	//Send "move origin" command
	send_word = 0xffd6;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED1_USART0(send_char_hi);
	TxD_OLED1_USART0(send_char_lo);
		
	//send xpos
	send_word = xpos;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED1_USART0(send_char_hi);
	TxD_OLED1_USART0(send_char_lo);
		
	//send ypos
	send_word = ypos;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED1_USART0(send_char_hi);
	TxD_OLED1_USART0(send_char_lo);

	oled1_busy_flag = 1;
	return;
}



/***********************************************************************
* oled2_setxt_position  subroutine (Uses MOVE ORIGIN serial command)
*
* Inputs: y-pos int, x-pos int
*
***********************************************************************/
void oled2_setxt_position (const uint16_t xpos, const uint16_t ypos)
{
	uint16_t send_word;
	uint8_t send_char_lo, send_char_hi; 

	// Wait for any executing commands to end...
	while (oled2_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled2_busy_flag == 1) && (USART3_STATUS & PIN7_bm)) { 
			if ((USART3.RXDATAL) == 0x06) {
				oled2_busy_flag = 0;
				oled2_NAK_flag = 0;
				}
			else { // NOT ACK, but NAK!
				oled2_NAK_flag = 1;
				}
			}
		}		

		//Send "move origin" command
		send_word = 0xffd6;
		send_char_hi = (char)((send_word & 0xff00) >> 8);
		send_char_lo = (char)(send_word & 0xff);
		TxD_OLED2_USART3(send_char_hi);
		TxD_OLED2_USART3(send_char_lo);
		
		//xpos
		send_word = xpos;
		send_char_hi = (char)((send_word & 0xff00) >> 8);
		send_char_lo = (char)(send_word & 0xff);
		TxD_OLED2_USART3(send_char_hi);
		TxD_OLED2_USART3(send_char_lo);
		
		//ypos
		send_word = ypos;
		send_char_hi = (char)((send_word & 0xff00) >> 8);
		send_char_lo = (char)(send_word & 0xff);
		TxD_OLED2_USART3(send_char_hi);
		TxD_OLED2_USART3(send_char_lo);

	oled2_busy_flag = 1;
	return;
}


/***********************************************************************
* oled3_setxt_position (Uses MOVE ORIGIN serial command)
*
* Inputs: y-pos int, x-pos int
* [USART1]
***********************************************************************/
void oled3_setxt_position (const uint16_t xpos, const uint16_t ypos)
{
	uint16_t send_word;
	uint8_t send_char_lo, send_char_hi; 

	// Wait for any executing commands to end...
	while (oled3_busy_flag == 1) {   // Wait for RESPONSEs char to arrive
		if ((oled3_busy_flag == 1) && (USART1_STATUS & PIN7_bm)) {  
			if ((USART1.RXDATAL) == 0x06) {
				oled3_busy_flag = 0;
				oled3_NAK_flag = 0;
			}
			else { // NOT ACK, but NAK!
				oled3_NAK_flag = 1;
			}
		}
	}
			
	//Send "move origin" command
	send_word = 0xffd6;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED3_USART1(send_char_hi);
	TxD_OLED3_USART1(send_char_lo);
		
	//xpos
	send_word = xpos;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED3_USART1(send_char_hi);
	TxD_OLED3_USART1(send_char_lo);
		
	// ypos
	send_word = ypos;
	send_char_hi = (char)((send_word & 0xff00) >> 8);
	send_char_lo = (char)(send_word & 0xff);
	TxD_OLED3_USART1(send_char_hi);
	TxD_OLED3_USART1(send_char_lo);

	oled3_busy_flag = 1;
	return;
}


/***********************************************************************
* void  ssc_oled_lines (void);
*
* Description: Draws all horizontal lines for SSC startup screens.
*	           Writes to all three displays.
*
***********************************************************************/
void ssc_oled_lines (void)
{

	// Add TOP lines
	oled1_send_command (&oled_drw_line_top[0]);
	oled2_send_command (&oled_drw_line_top[0]);
	oled3_send_command (&oled_drw_line_top[0]);
	
	oled1_send_command (&oled_drw_line_top2[0]);
	oled2_send_command (&oled_drw_line_top2[0]);
	oled3_send_command (&oled_drw_line_top2[0]);
	
	oled1_send_command (&oled_drw_line_top3[0]);
	oled2_send_command (&oled_drw_line_top3[0]);
	oled3_send_command (&oled_drw_line_top3[0]);

	// Add bottom lines
	oled1_send_command (&oled_drw_line_bottom[0]);
	oled2_send_command (&oled_drw_line_bottom[0]);
	oled3_send_command (&oled_drw_line_bottom[0]);

	oled1_send_command (&oled_drw_line_bottom2[0]);
	oled2_send_command (&oled_drw_line_bottom2[0]);
	oled3_send_command (&oled_drw_line_bottom2[0]);
	
	oled1_send_command (&oled_drw_line_bottom3[0]);
	oled2_send_command (&oled_drw_line_bottom3[0]);
	oled3_send_command (&oled_drw_line_bottom3[0]);
}


/* ================================================================
 * ================================================================
 * OLEDS   C L E A R   T E X T   A R E A    routines
 * ================================================================
 * ============================================================= */

// Clears the entire text/dsp area in middle OLED
// but does NOT erase top and bottom lines!!

void clr_text_area1 (void)  //oled1 USART0
{
		oled1_setxt_position (1,6);   // was 1,4
	oled1_send_command (&oled_setxt_height_talltall[0]);  // set text height
	oled1_putstring (&ClrTextArea[0]);    //
	oled1_send_command (&oled_setxt_height[0]);  // set text height
		///	oled1_busy_flag = 1; // DONE in EACH routine!
	return;
}


void clr_text_area2 (void)  //oled2 USART3
{	
	oled2_setxt_position (1,6);                          // Clr the FIVE from right screen,
	oled2_send_command (&oled_setxt_height_talltall[0]);    // set text height
	oled2_putstring (&ClrTextArea[0]);    // (wait_for-FIVE-seconds msg)
	oled2_send_command (&oled_setxt_height[0]);  // set text height
	return;
}



void clr_text_area2_short (void)  //oled2 USART3
{
	oled2_setxt_position (4,10);  
	oled2_send_command (&oled_setxt_width_wide3[0]);  // set text height
	oled2_send_command (&oled_setxt_height_talltall[0]);  // set text height
	oled2_putstring (&ClrTextAreaShort[0]);    // (wait_for-FIVE-seconds msg)
	oled2_send_command (&oled_setxt_height[0]);  // set text height
		///	oled2_busy_flag = 1;  // DONE in EACH routine!
	return;
}


void clr_text_area3 (void)  //oled3 USART1
{
	oled3_setxt_position (1,4);   // Clr the FIVE from right screen,
	oled3_send_command (&oled_setxt_height_talltall[0]);  // set text height
	oled3_putstring (&ClrTextArea[0]);    // (wait_for-FIVE-seconds msg)
	oled3_send_command (&oled_setxt_height[0]);  // set text height
		///oled3_busy_flag = 1;
	return;
}


void clr_all_text_areas (void)
{
	//Clear ALL OLED Text Areas (leavint lines!)
	oled1_setxt_position (1,4);   // Clr start position
	oled2_setxt_position (1,4);   // Clr start position
	oled3_setxt_position (1,4);   // Clr start position

	oled1_send_command (&oled_setxt_height_talltall[0]);  // set text height
	oled2_send_command (&oled_setxt_height_talltall[0]);  // set text height
	oled3_send_command (&oled_setxt_height_talltall[0]);  // set text height

	oled1_putstring (&ClrTextArea[0]);    //
	oled2_putstring (&ClrTextArea[0]);    //
	oled3_putstring (&ClrTextArea[0]);    //
}
