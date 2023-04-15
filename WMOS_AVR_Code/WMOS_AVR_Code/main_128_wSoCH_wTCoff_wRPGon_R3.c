/* EVIM wSSC based on AVR128.NOseatSENSOR.wTimerTIMOUT.wRPG-on-Feature
 * (This version includes the RPG-On Turn On Feature)
 * ---------------------
 * Created:   Early 2023 (Winter-Spring) 
 * Modified:  Last update before the addition of the WiFi Routines)
 * Version:   Release 03
 * Author:    F S Tierno
 * 
 * **************************************************************
 * ** Timer RPG Version ** ENHANCED ** KEYIN and IGN TRIGGERED
 * **************************************************************
 *
 *  RELEASE 02C ===   A DEPLOYED VERSION  ===  Winter 2022
 *  RELEASE 03 ===   WiFi Development Version Winter-Spring 2023
 *
 *  
 * CORRECTIONS SUMMARY (Update History)
 * -------------------------------
 *  1. Fixed kWh inaccuracy (was 1/10 the correct value)
 *
 *  2. Added the SoCH detect subroutine and "soc_offline_flag"
 *
 *  3. Setup initial default values for all parameter arrays
 *     (Pack-V, Pack-I, SOC%, kWh)  Useful if SDT unit fails
 *
 *  4. Powers up accelerator pedal-lock on door opening
 *     (resets w/door closed)
 *
 *  5. Power to ped-lock mech now stays ON unless/until:
 *         1. Door closed, and in STBY mode
 *         2. In EVIM state (instrumentation mode)
 *
 *  6. Adjusted correction offset for 13.3VDC accy
 *     battery voltage (now -0.02 VDC)
 *
 *  7. Set fuse to enable external reset PB switch
 *     (Reset input PF6 pullup also enabled!)
 *
 *  8. Increased STBY LED flash rate, and decreased strobe duration
 *
 *  9. Added the manual SoCH reset: Press "underdash" reset PBSW, while
 *     pressing and holding RPG-PB, until three short beeps are heard
 *
 * 10. Modified Mode-Change to only alter the MIDDLE OLED2 display
 *     (Includes update of the OLED_InterfaceRoutings_AVR128_Release3.inc
 *      and ISR_InterfaceRoutings_AVR128_Release3.inc)
 *
 * 11. Powers up Display Modules with with DOOR Open *OR* with 
 *     KEY inserted activity (with ~5 minute timeout).
 *
 * 12. Added timeout TCA0 for return to STDBY after a pre-defined delay
 *     period... Returns system to STBY State from WAKE1 State.
 *
 * 13. Added RPG-turn-on feature: 1. Open drivers side door; and 3. press
 *     RPG... Times out after approximately one minute).
 *
 * --------   Remote Access Changes   -------------------------------
 *
 * 14. Added routines and code to initialized the "globals" for wi-fi 
 *     remote access.  Added routings includes load_133V_battery_voltage(),
 *     load_a12V_voltage(), and load_a5V_voltage.
 *
 * 15. Also added code to load all voltages, currents, etc., when entering 
 *     the "pseudo" WAKE1 STATE.  This causes the following when to 
 *     occur when entering the pseudo state:
 *         a) Loads the 13.3V, the a12V, and the a5V digit values in
 *            the globals variables;
 *         b) Loads global variables for the Drive Motor temp, the DC-DC
 *            converter heat sink temp, the DC Controller air output
 *            temp, as well as the two battery box air temps and the
 *            outside ambient temp.
 *         c) Loads the pack-V, pack-I, pack-SoC, and the pack kWhs used
 *            into global variables. 
 *            output temp.
 *		   d) TCA0 set for longer timeout (~ 5 minutes).  Also, TCA0 
 *			  counter register is reset to zero with each new data request.
 *
 *   -------------------------------------------------------------------
 *   Basic Comm Init Routine is for all 4 UARTs
 *
 *	  USART2 = Serial channel to SDT module for SOCH
 *    USART0 = Left OLED LCD
 *    USART3 = Middle OLED LCD
 *    USART1 = Right OLED LCD
 *	  
 *
 *  uOLED-160-G2 Specs
 *  ------------------
 *   Low Cost 1.7" OLED graphics display
 *   OPERATING in Serial Command Mode (USART Interface)
 *   Resolution:  160 x 128 with origin in upper left corner
 *                so that (0-159 across, and 0-127 down)
 *   Color: RGB, 65K True Color
 *   Goldelox Processor (Intelligent Graphics Module)
 *   uSD Card Holder (4GB and above)
 *   5-wire interface
 *          Pin-1 = +5V Input
 *          Pin-3 = TX (Serial Data Out)
 *          Pin-5 = RX (Serial Data In)
 *          Pin-7 = GND
 *          Pin-9 = Reset (Active Low, 2 uS minimum)
 *   Viewing Angle: >160 degrees.
 *   Contrast Ratio... 5000:1
 *
 *
 *   Vehicle INTERRUPT inputs (along with active/passive levels
 *   ----------------------------------------------------------
 *		DOOR  (PE1)   1==Closed;  0==Open
 *		KEY   (PE2)   1==Keyin;   0=Keyout
 *		CHRG Mode (PA4 = 1 == Chrg Mode)
 *
 *      PWR_SVO (PA6)    1=ON;  0=Off
 *      POWER_SoC (PF3)  1=ON;  0=Off
 *
 *
 *  Current Version Reduced FSM States/Pseudo States :
 *    1.  Standby :  OLEDs and all peripherals powered down;
 *					 Generates an "Alarm Active" LED strobe while
 *                   in this mode (Driver door must be closed)
 *
 *    2a. Wait1   :  DOOR *or* Keyin powers up OLEDs and waits for IGN
 *                   to flash the WAIT message.
 *	  2b. Ignition Voltage Present: Updates center OLED, Flashes WAIT
 *
 *    3.  EVIM: Unlocks Accelerator Pedal      
 *				Instrumentation/Operating State - Displays the following: 
 *                OLED1/Left: Accessory battery voltage
 *                OLED2/Center: Pack-V, Pack-I, SoC%, kWhs along with
 *                              Aux12V and Aux5V levels (sml bottom);
 *                OLED3/Right: Temperature Stations
 *                     (Motor, Cntrlr, DC-DC, BBox1, Bbox2, Ambient)
 *
 */


#define F_CPU 8000000UL  // 8 MHz

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>

#define USART_BAUD_RATE(BAUD_RATE) ((float)(8000000*64/(16*(float)BAUD_RATE)) + 0.5)

// Define Top Level FSM states for SSC Power Up Sequence
#define STANDBY_STATE   0      // Only '128 CPU powered up
#define WAKE1_STATE     1      // ** All Peripherals powered up
#define IGNITION_STATE  4      // ** Third/Final WAIT-flash screens
#define EVIM_STATE      5      // Run-state, V, I, and T info

// Notes:
//   ** These are Pseudo states
//   EVIM_STATE also == Charge State!

// Define EVIM_State temperature "lower" FSM states
#define MOTOR_STATE 5        // Motor housing temp
#define CONTROLLER_STATE 4   // Air output temp
#define DCDC_STATE  3        // DCDC Conv heatsink plate
#define BBOX1_STATE 2        // Mid-car battery box
#define BBOX2_STATE 1        // Rear/Trunk battery box
#define AMBIENT_STATE 0      // Outside air temp (front motor compartment)

#define stobe_cnt_value 1600   // stobe count value (when ==, = flash LED)


// Subroutine Prototypes....
// Subroutine Prototypes....
// Subroutine Prototypes....
// -----------------------------------------------------------------------
void fcpu_init(void);

void motor_tasks (void);
void controller_tasks (void);
void dcdc_tasks (void);
void bbox1_tasks (void);
void bbox2_tasks (void);
void ambient_tasks (void);
void battery_tasks (void);
void temp_high_tasks (void);
void temp_low_tasks (void);

void USARTs_Init(void);

void clr_tmp_dsp_area (void);
void dsp_ps_temp ( int16_t temp2disp);
void snd_serial_cmd (uint8_t cmd_byte);
void load_evim_screen_lines (void);
void load_tmparray_display (void);
void load_accy_voltage_display_init(void);
void oled_init (void);
void oled_set_def_text (void);

void tone (void);

void clr_all_text_areas (void);
void ssc_oled_lines (void);
void wait_state_screen_loads (void);
void keyin_state_screen_loads (void);
void ignition_state_screen_loads (void);
void entering_evim_state_screen_loads(void);

void ADC0_init (void);
void get_temps(void);
int16_t get_adc_ntc10k (uint16_t channel_number);
int16_t scale_temp_ntc10k ( int16_t adc_result_12bit);
void get_133V_battery_voltage (void);
void get_a12V_voltage (void);
void get_a5V_voltage (void);

void scale_temps_array(void);

//SOC RELATED...
uint8_t USART2_RcvChr(void);
void USART2_SndChr(uint8_t c);
void SOC_UART2_SndCmd (const uint8_t *array_ptr);
void verify_SOCH_online(void);

void display_pack_current (void);
void display_pack_volatage (void);
void display_pack_kWh (void);
void display_pack_soc (void);
void soc_display_init_load (void);

void unlock_accel_pedal_slo(void);
void lock_accel_pedal_slo(void);
void pedal_lock_pwr_on(void);
void pedal_lock_pwr_off(void);

void TCA0_init(void);  // Startup routine
void TCA0_stop (void);  // Stop routine




// -----------------------------------------------------------
// -----------------------------------------------------------
// GLOBAL VARIABLE DEFINITIONS SECTION
// GLOBAL VARIABLE DEFINITIONS SECTION
// GLOBAL VARIABLE DEFINITIONS SECTION
// -----------------------------------------------------------
// -----------------------------------------------------------


// SoC Head unit storage array definitions...
//----------------------------------------------------------

// Pack voltage storage array (w/init value, showing format)
uint8_t pack_voltage_array [16] = {" 60V 176.54V"};
     //uint8_t pack_voltage_array [16] = {0};

// Pack current storage array (w/init value, showing format)
uint8_t pack_current_array [16] = {" 60C +0012.3A"};
    //uint8_t pack_current_array [16] = {0};

// Pack state-of-charge storage array
uint8_t pack_soc_array [16] = {" 60G 98.7%"};
    //uint8_t pack_soc_array [16] = {0};

// Pack kWh storage array
uint8_t pack_kwh_array [16]   = {" 60WH -04321.1WH"};
    //uint8_t pack_kwh_array [16] = {0};



// Low voltage "digit" storage definitions
//-----------------------------------------------------

// Global Variables for the Aux-5V (from HV)
uint16_t aux5_hundredths;
uint16_t aux5_tenths;
uint16_t aux5_units;
uint16_t aux5_tens;

// Global Variables for the Aux-12V (from HV)
uint16_t aux12_hundredths;
uint16_t aux12_tenths;
uint16_t aux12_units;
uint16_t aux12_tens;

// Global Variables for the Accy 13.3V battery
uint16_t accy133_hundredths;
uint16_t accy133_tenths;
uint16_t accy133_units;
uint16_t accy133_tens;
 	   

// RAW temp sensor data storage array, and the scaled
// temperature array
// Channel 0 == Ambient temp
//         1 == BBox2 internal temp
//         2 == BBox1 internal temp
//         3 == Ambient temp
//         4 == DC Controller (output) air temp
//         5 == DC Drive Motor temp
// ------------------------------------------------------------------
int16_t current_temps_array [6] = {0};  // current/last temp readings.
int16_t scaled_temps_array [6] = {0};   // current/last scaled temps in 
                                  // tenths of a degree.

// Storage for the "current" accessory battery voltage
uint16_t current_accy_batt_voltage = 0;  //accy batt voltage
		   
// Global timer counter (Short and longer timeout for return to STANDBY
uint16_t to_counter = 0;  //USART2 receive timeout ctr

// General use GLOBAL variables
uint8_t i;
uint16_t dly_val;

// Start up state for EVIM mode
uint8_t state_num = 4;    // DC Controller

// Startup Sequence Controller State Number
uint8_t top_state_num = 0;  // STANDBY state

// Port images - for checking multiple bits on one port
uint8_t portB_image_old, temp_port_image;

uint8_t contrast_level = 1;  // DEBUG=3... init val = 1 = LOW power;
uint8_t contrast_oldval;

uint8_t first_time = 0;
uint8_t serial_command_byte;   
uint8_t charge_cycle_active_flag; 
uint8_t rpg_on_flag = 0;


uint8_t dsp_mode_flag = 0;    //0122-2022 pm
uint8_t dsp_mode_sw_val;		// current value of MODE-SW
uint8_t mode_changed = 0;  // set active
uint8_t mode_switch_counter = 0;

uint8_t direction_flag=0;  // 0 = CCW, otherwise CW...
uint16_t current_state = 0;   // MOTOR_STATE;
uint16_t receive_word = 0;     

int16_t current_temps_array [6];  // current/last temp readings
int16_t scaled_temps_array [6];   // current/last scaled temps

uint16_t current_accy_batt_voltage;  //accy batt voltage  

//OLED BUSY & NAK FLAGS - Version 7 Addition for || processing 
uint8_t oled1_busy_flag;  // When a '1', must wait for executing
uint8_t oled2_busy_flag;  // command to end (which will reset
uint8_t oled3_busy_flag;  // the flag back to '0'...

uint8_t oled1_NAK_flag;   // When a '1', must wait for executing
uint8_t oled2_NAK_flag;   // command to end (which will reset
uint8_t oled3_NAK_flag;   // the flag back to '0'...

// Required STATE and Status flags...
uint8_t standby_state_active_flag;
uint8_t evim_state_active_flag;
uint8_t warm_restart_flag;
uint8_t tailite_flag;   // Moved here 0n 09/14/2022

uint8_t count;    // DEBUG VAR
uint8_t first_response_char;

//  ISR RELATED VARIABLES
uint8_t PORTA_image = 0;        // PortD image at the occurrence of intr
uint8_t portB_image = 0;        // PortD image at the occurrence of intr
uint8_t PORTC_image = 0;        // PortC image at the occurrence of intr
uint8_t PORTE_image = 0;        // PortE image at the occurrence of intr

uint8_t new_dsp_mode_sw_val = 0;
uint8_t temp_port_image = 0;    //
uint8_t channel_d3_d2 = 0;      // char to hold ChA (d6) & ChB (d7) values

// TCA0 TIMEOUT Cnt-val (for return to STANDBY_STATE)
uint8_t timeout_counter = 0;  // 0-Ovr-Flow is about 6.5 seconds

// flag for ambient state active
uint8_t ambient_active = 0;   //     ...  1=ambient;  0=normal
uint16_t interrupted_state = 0;     // holds state to return


// SOC Head Related Flags 
// ----------------------------------------------                                                         
uint8_t pack_voltage_flag = 0; //if 1 == pck voltage at ~182 VDC & UP!
uint8_t pack_soc95_flag = 0;   //if 1 == SoC >= 95%                         
uint8_t soch_offline_flag = 0; // 1 = SOCH Off-line; 0 = SOCH online/ready

	  

/*********************************************************************
 void tone (void)
   Description: Generates a short beep/tone using PB5
                (Active high)
********************************************************************/
void tone (void)
 {   
	 PORTB.OUTSET = PIN5_bm;
     _delay_us(1500);  
	 PORTB.OUTCLR = PIN5_bm;
 }


/***********************************************************************
* USARTs_Init     
* Description: Initializes all USART for Async operation
*
*   Sets TxD port bits to OUTPUT (PA0, PC0, PF4*, PB0)
*                                (Tx0, Tx1, Tx2, Tx3)  
*   Sets USART2 (USB) to 9600:8:N:1 (baud: data bits: No parity: 1 stop)
*   Sets USART0, USART1, and USART3 to 38400:8:N:1
*
*      * PF4 is the alternate pin position
***********************************************************************/
void USARTs_Init (void)
{
	// Set TxD pins to OUTPUT
	PORTA.DIRSET = PIN0_bm;   // Set PA0 output (TxD0 OLED1 left)
	PORTB.DIRSET = PIN0_bm;   // Set PB0 output (TxD3 OLED2 center)
	PORTC.DIRSET = PIN0_bm;   // Set PC0 output (TxD1 OLED3 right)
	PORTF.DIRSET = PIN4_bm;   // Set PF4 output (TxD2 USB SOC)
	
	// Set USART2 (USB Channel) to alternate pins positions...
    //set bit 4 to 1 (for alt pin assign)
	PORTMUX.USARTROUTEA |= PORTMUX_USART2_ALT1_gc; 

	// Set BAUD rate for USART2 == USB Channel to SOC Head
	USART2.BAUD = (uint16_t) USART_BAUD_RATE (9600); 

	// Set BAUD rate for three OLED Channels
	// uOLED-160-G2 OLEDS:  Left,  center,  right     
	USART0.BAUD = (uint16_t) USART_BAUD_RATE (38400);   // Left OLED 
	USART3.BAUD = (uint16_t) USART_BAUD_RATE (38400);   // Center OLED
	USART1.BAUD = (uint16_t) USART_BAUD_RATE (38400);   // Right OLED
	 
	// Enable all four USARTs
	USART0.CTRLB |= USART_TXEN_bm;    // enable transmitter
	USART0.CTRLB |= USART_RXEN_bm;    // enable receiver
	USART1.CTRLB |= USART_TXEN_bm;
	USART1.CTRLB |= USART_RXEN_bm;
	USART2.CTRLB |= USART_TXEN_bm;
	USART2.CTRLB |= USART_RXEN_bm;
	USART3.CTRLB |= USART_TXEN_bm;
	USART3.CTRLB |= USART_RXEN_bm;
}



////////////////////////////////////////
////  INCLUDE FILE FOR ALL OLED ROUTINES
////  ----------------------------------
# include <OLED.h>
#include <OLED_InterfaceRoutines_AVR128_Release3.inc>
////////////////////////////////////////
////////////////////////////////////////



//=========================================================================
//===========  STATE OF CHARGE (SOC) HEAD ROUTINES   ======================
//=========================================================================

/**********************************************************************
USART2 RECEIVE CHARACTER ROUTINE [Data from SOC Head (SOCH)]
**********************************************************************
* uint8_t USART2_RcvChr2 (void);
* Description:	Waits for: "RXCIF" to be set, and then returns...
*				the "RXDATAL" byte/character...
**********************************************************************/
uint8_t USART2_RcvChr(void)
{
	while (!(USART2.STATUS & USART_RXCIF_bm)) {};
	return (USART2.RXDATAL);
}


/**********************************************************************
USART2 SENDS CHARS TO SOCH
**********************************************************************
* USART2_SndChr (uint8_t c);
*
* Description: Verifies the transmit DATA register is empty, then
*              loads "c" into the data transmit register.
***********************************************************************/
void USART2_SndChr (uint8_t c)
{
	while (!(USART2.STATUS & USART_DREIF_bm)) {
		; // wait for DR to empty
		}
	USART2.TXDATAL = c;
}


// SOC COMMAND CONSTANT STRINGS AND ROUTINES
//*********************************************************************
// REMEMBER.... ADD 13 == CR (To end of commands)

// get-voltage command string
const uint8_t get_pack_voltage[6]  = {5,'6','0','v','.', 13};

// get-current command
const uint8_t get_pack_current[6]  = {5,'6','0','c','.', 13};	

// get-SOC/Gauge command **
const uint8_t get_pack_soc[6]	   = {5,'6','0','g','.', 13};

// get watt hours used ** 			
const uint8_t get_watthours_soc[6] = {5,'6','0','w','.', 13};

// reset command string (clrs all 'accumulative' settings)
// (best issued after a charge session of some level or duration)
const uint8_t soc_reset[6]         = {5,'6','0','r','.', 13};

// SELF TEST RELATED...
// Verification of SOCH command (set current range, read only)
const uint8_t soch_verify_command [7] = {6,'6','0','s','e','.', 13};





/*********************************************************************
* SOC Head Send Data-Request Command Routine
* SOC_UART2_SndCmd (uint8_t c);
*
* Description: Send the command string pointed to by *array_ptr 
*              
***********************************************************************/
void SOC_UART2_SndCmd (const uint8_t *array_ptr)
{
	uint8_t i, length;   
	uint8_t send_char;
	const uint8_t *ptr;  // pointer to command to send

	ptr = array_ptr;   // load pointer for command string
	i = 0;
	length = *ptr++;   //load command length value = 1st byte

	while (i < length) //length == byte count
	{
		send_char = *ptr++;
		USART2_SndChr (send_char);
		i = i +1;
	}
}


/*********************************************************************
* Get_SOC_Response (array_ptr, end_char);
*
* Description:
* -----------
*   Reads a response from the SOC head, including, V, I, SOC%, and kWh;  
*   Stores response in array pointed to by array_ptr.
***********************************************************************/
void Get_SOC_Response (uint8_t *array_ptr, uint8_t EOS_char)
{
	uint8_t EOSCHR; // V = volts, A = current, % = SOC, W(H)= watts
	uint8_t i;
	uint8_t *ptr;  // pointer to strt of cmd string to send
	uint8_t recv_char = 0;

	ptr = array_ptr;  // load pointer for command string
	EOSCHR = EOS_char;  //end of str char, == end of response
	
		
	//clear pending chars - VERY NECESSARY   Why??? Not sure
	recv_char = USART2.RXDATAL;  // clear spurious data !!!!!!!
	recv_char = USART2.RXDATAL;  // clear spurious data !!
	recv_char = USART2.RXDATAL;  // clear spurious data !!
		
	// get first FIVE response chars and STORE in buffer
	for (i =0; i<5; i++) {
		recv_char = USART2_RcvChr();  
		*ptr++ = recv_char;   //store received response char
		}
	
	recv_char = 0;  //
	// get rest of response string == actual value in ASCII, with units!!
	while ( EOSCHR != recv_char)
	{
		recv_char = USART2_RcvChr();  // get 1st/next char
		*ptr++ = recv_char;   //store received response char
		}
}



/*********************************************************************
* verify_SOCH_online(void);
*
* Description:
* -----------
*   Sends a test command to the SOCH and checks for the response chars,
*   WITH a timeout counter included... If response received, sets the
*   "soch_offline_flag" to 0.
*
*   If no response is received from the SOCH in ~ 2mS, the
*   soch_offline_flag is set to 1, and returns.
*
***********************************************************************/
void verify_SOCH_online(void)
{
	uint8_t i;
	uint8_t recv_char = 0;
	
	to_counter = 0;
	//send command to verify SOCH is there...
	SOC_UART2_SndCmd (&soch_verify_command[0]);
	//SOC_UART2_SndCmd (&get_pack_voltage[0]);
	recv_char = USART2.RXDATAL;  // clear spurious data !!!!!!!
	recv_char = USART2.RXDATAL;  // clear spurious data !!!!!!!
	recv_char = USART2.RXDATAL;  // clear spurious data !!!!!!!

	// Wait for response, with timeout counter...
	// get response chars and discard...
	for (i=0; i<7; i++) {
	  while ((!(USART2.STATUS & USART_RXCIF_bm)) && (to_counter < 0x123)) {
		_delay_us(3);
		to_counter++;
		}
	recv_char = USART2.RXDATAL;  // 
	}

	if (to_counter >= 0x123) {
		soch_offline_flag = 1;     // SOCH is ready
		}
	else {
		soch_offline_flag = 0;  // SOCH is OFF LINE
		}
	}



/*********************************************************************
* display_pack_voltage (array_ptr, end_char);
*
* Description: Reads the value in the uint8_t "pack_voltage_array",
*              which is 16 bytes total, and displays the value, in
*              volts on the middle OLED2.
*
***********************************************************************/
void display_pack_voltage (void)
{
	uint8_t string_char = '0', first_digit = 0;
	const uint8_t *ptr;  // pointer to command to send
	
	ptr = &pack_voltage_array[4];

	// OK, set up OLED2..
	oled2_setxt_position (37,18); // Loc for start of value digits
	oled2_send_command (&oled_setxt_width_wide3[0]);
	oled2_send_command(&oled_setxt_height_tall[0]);
	first_digit = 0;  // set this when first non-zero digit found
	
	while (first_digit == 0) {
		string_char = *ptr++;  // get 1st/next char
		if ((string_char == '0') || (string_char == 0x56))
		{ /*do nothing */ }  // leading 0... Discard
		else if ((string_char >= '1') && (string_char <= '9')) {  
			putcharOLED2(string_char);
			first_digit = 1;
		}
		else if (string_char == '.') {  // minus sign?
			//putcharOLED2(' ');
			putcharOLED2('0');
			putcharOLED2(string_char); // display "."
			string_char = *ptr++;  // get 10ths digit
			putcharOLED2(string_char); // display 10ths
			putcharOLED2(' ');
			first_digit = 1;
		}
	}

	// send value chars, but only to the TENTHs of a volt.
	// End of string is an 'V'
	while (string_char != 'V') {
		string_char = *ptr++;  // get 1st/next char
		if (string_char == '.') {  // just send "V"
			ptr = ptr + 2; 
			// Settings for 'V', last pass through outer loop
			oled2_send_command(&oled_setxt_height[0]);
			oled2_setxt_position (114,23);   //   was 115
		}
		else
		putcharOLED2(string_char);  // output next digit
	}									// (and 'V' at end!)
}



/*********************************************************************
* display_pack_current (void);
*
* Description: Reads the value in the uint8_t "pack_current_array",
*              which is 16 bytes total, and displays the value, in
*              volts on the middle OLED2.
*
***********************************************************************/
void display_pack_current (void)
{
	uint8_t string_char = '0';
	uint8_t minus_found = 0;
	const uint8_t *ptr;  // pointer to command to send
	
	uint8_t thousands = 0;
	uint8_t hundreds = 0;
	uint8_t tens = 0;
	uint8_t units = 0;
	uint8_t tenths = 0;
	
	// Scan first portion of array for a minus "-" sign
	ptr = &pack_current_array[0];
	while ((string_char != '-') && (count <= 10)) {
		string_char = *ptr++;  // get 1st/next char
		}
	
	// Test for MINUS sign, and don't display if found
	//     If *no* MINUS... send OLED2 a "+" sign...
	if (string_char == '-') { //Then found, send...
		minus_found = 1;
		}
	else {  // NO MINUS
		minus_found = 0;
	    oled2_setxt_position (5,78); // position for minus
		putcharOLED2('+');
		} 



	// N E E D E D      N E E D E D
	//
	// ================================================================
	// Process digits based on values...
	// If value 9.9 or less, display 1/10ths ELSE
	//    if value >=10, display amps only
	// ================================================================

	// Determine array digits start location (Based on minus-found flag)
	if (minus_found == 1) {
		ptr = &pack_current_array[6];
		}
	else {
		ptr = &pack_current_array[5];
		}

	// load digits for analysis/processing	
	thousands = *ptr++;
	hundreds = *ptr++;
	tens = *ptr++;
	units = *ptr++;
	ptr++;  // skip the decimal point!!
	tenths = *ptr++;

    oled2_setxt_position (37,78); // STARTing location for digits
	oled2_send_command (&oled_setxt_width_wide3[0]);
	oled2_send_command(&oled_setxt_height_tall[0]); 

	if ((thousands == '0') && (hundreds == '0') && (tens == '0')) {
		// Just units and tenths To display
		putcharOLED2(units);
		putcharOLED2('.');
		putcharOLED2(tenths);
		}
	else if ((thousands == '0') && (hundreds == '0')) {
		// Just Tens, units to display (NO decimal point!)
		putcharOLED2(' ');  // space for hundreds position
		putcharOLED2(tens);
		putcharOLED2(units);
		}
	else if (thousands == '0') {
		// Then display hundreds, tens, and units (NO decimal point!)
		putcharOLED2(hundreds);  // space for hundreds position
		putcharOLED2(tens);
		putcharOLED2(units);
		}

	// Send the 'A'
		oled2_send_command(&oled_setxt_height[0]);
		oled2_setxt_position (114,83);   //   was 115
		putcharOLED2('A');  
}


/*********************************************************************
* display_pack_SoC (array_ptr, end_char);
*
* Description: Reads the value in the uint8_t "pack_soc_array",
*              which is 16 bytes total, and displays the value, in
*              volts on the middle OLED2.
*
***********************************************************************/
void display_pack_soc (void)
{
	uint8_t string_char = '0', first_digit = 0;
	const uint8_t *ptr;  // pointer to command to send
	
	ptr = &pack_soc_array[4];  // set to first digit of soc percentage

	// OK, set up OLED2..
	oled2_setxt_position (23,55); // Loc for start of value digits
	oled2_send_command (&oled_setxt_width[0]);
	oled2_send_command(&oled_setxt_height[0]);
	first_digit = 0;  // set this when first non-zero digit found
	
	while (first_digit == 0) {
		string_char = *ptr++;  // get 1st/next char
		if (string_char == '0')
		{ /*do nothing */ }  // leading 0... Discard
		else if ((string_char >= '1') && (string_char <= '9')) {  
			putcharOLED2(string_char);
			first_digit = 1;
		}
	}

	// send value chars, but only to the TENTHs of a volt.
	// End of string is '%'
	while (string_char != '%') {
		string_char = *ptr++;  // get 1st/next char
		if (string_char == '.') { // snd it & the 10ths digit
			string_char = *ptr++;  // get 10ths digit
					
			// Settings for '%', last pass through outer loop
			oled2_send_command(&oled_setxt_height_med[0]);
			oled2_setxt_position (52,59);  // % position
		}
		else
		putcharOLED2(string_char);  // output next digit
	}									// (and '%' at end!)
}


/*********************************************************************
* display_pack_kwh (array_ptr, end_char);
*
* Description: Reads the value in the uint8_t "pack_kwh_array",
*              which is 16 bytes total, and displays the value, in
*              in kilowatts on the middle OLED2.
*
***********************************************************************/
void display_pack_kwh (void)
{
	uint8_t string_char = '0';  
	const uint8_t *ptr;  // pointer to command to send
	
	ptr = &pack_kwh_array[8];   // pointing to first digit of interest (kWh units)

	// OK, set up OLED2 for kWh value...
	oled2_setxt_position (92,55); // Loc for start of value digits
	oled2_send_command (&oled_setxt_width[0]);
	oled2_send_command(&oled_setxt_height[0]);

	// send first digit...
	string_char = *ptr++;  // get kWh units (0-9) digit...
	putcharOLED2(string_char);

	// send decimal point...
	string_char = '.';  // put the decimal out
	oled2_send_command (&oled_setxt_width_narrow[0]);
	putcharOLED2(string_char);

	oled2_send_command(&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_width[0]);
	oled2_setxt_position (114,55); // Loc for start of tenths digit

	// send tenths digit
	string_char = *ptr++;  // get kWh digit...
	putcharOLED2(string_char);

	// send hundredths digit
	string_char = *ptr++;  // get kWh digit...
	putcharOLED2(string_char);
	}



//PACK VOLTAGE FLAG : EVALUATING AND SETTING/CLRING ROUTINE 
// **********************************************************************
// void check_pack_magnitude (void)
//
// Description: Using the hundreds, tens, and units digits of the 
//     current pack voltage "@pack_voltage_array[4]plus"
//
//     So the "total"  (t) is determined by:
//     total = 100*hundreds + 10*tens + units
//
// Output: pack_voltage_flag == 1 if V(pack) >= 180 VDC
//                           == 0 if V(pack) < 180 VDC
// 
// **********************************************************************
void check_pack_magnitude (void)
{
	uint8_t total, partialtotal;  // NEVER more than 255!
	const uint8_t *ptr;  // pointer to command to send

	// discard blank space (0x20)
	//*ptr++;
		
	ptr = &pack_voltage_array[5];  //set ptr to hundreds digit value
	// get hundreds digit ASCII value and mult by digit weight (100)
	partialtotal = *ptr++;  
	partialtotal = partialtotal - 0x30;  // get digit only value
	total = 100*partialtotal;

	// get tens digit value and mult by digit weight (10)
	partialtotal = *ptr++;  
	partialtotal = partialtotal - 0x30;  // get digit only value
	total = total + 10*partialtotal; //
		
	// get units digit value and add in to total
	partialtotal = *ptr++;  
	partialtotal = partialtotal - 0x30;  // get digit only value
	total = total + partialtotal; //

	// Test value to set pack_voltage_flag == 1 
	// (if pack voltage 180 VDC or higher!)
	if (total >= 180) {
		pack_voltage_flag = 1;
		}
	else {
		pack_voltage_flag = 0;
		}
}






















//SOC 95% FLAG : EVALUATING AND SETTING/CLRING ROUTINE 
// **********************************************************************
// void check_soc_magnitude (void)
//
// Description: Using the hundreds, tens, and units digits of the
//     current pack voltage "@pack_voltage_array[4]plus"
//
//     So the "total"  (t) is determined by:
//     total = 100*hundreds + 10*tens + units
//
// Output: pack_voltage_flag == 1 if V(pack) >= 178 VDC
//                           == 0 if V(pack) < 178 VDC
//
// **********************************************************************
void check_soc_magnitude (void)
{
	uint8_t total, partialtotal;  // NEVER more than 255!
	const uint8_t *ptr;  // pointer to command to send
	
	ptr = &pack_soc_array[5];  //set ptr to hundreds digit value

	// get tens digit ASCII value and mult by digit weight (100)
	partialtotal = *ptr++;
	partialtotal = partialtotal - 0x30;  // get digit only value
	total = 10*partialtotal;

	// get units digit value and add in to total
	partialtotal = *ptr++;
	partialtotal = partialtotal - 0x30;  // get digit only value
	total = total + partialtotal; //

	// Test SOC value to set pack_soc_flag == 1 if pack soc 95% or higher!
	if (total >= 95) {
		pack_soc95_flag = 1;
	}
	else {
		pack_soc95_flag = 0;
	}
}



// --------------------------------------------------------------
// SOC/PWR MODE : Displays SOC per cento and the kWh used since last
//            charge event (MIDDLE OLED2)
// Description: Loads OLEDs with
//             SOC |  kWh
//           ----- | -----
//           82.4% |  2.37
//          ---------------  
//           12.3V | 5.34V
// --------------------------------------------------------------
void soc_state_screen_load (void)
{
	// clear the text area of OLED2
	clr_text_area2_short();   // was _short

	// Set text height and width for...
	oled2_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_width[0]);

	// Set first line position...
	oled2_setxt_position (21,17);

	// Write first lines
	oled2_putstring (&Soc_kWh[0]);

	// Write center vertical line...
	oled2_send_command (&oled_drw_vline_mid[0]);
	oled2_send_command (&oled_drw_hline_lo[0]);

}


//========================================================================
//========================================================================
//=======  S S C   S T A R T U P    M E S S A G E S    ===================
//========================================================================
//========================================================================


// entering_evim_state_screen_loads
// --------------------------------------------------------------
// USED WHEN ON THE WAY TO EVIM STATE
// Description: Loads OLEDs with
//       "Fasten"         "WAIT"        "Powering"
//     "Seat Belts"		Don't Press     "Vehicle"
//       			   "Accelerator"
// --------------------------------------------------------------
void entering_evim_state_screen_loads (void)
{
	// Clear the "accelerator" string
	oled1_setxt_position (15,10);   // Clr start position
	oled1_send_command (&oled_setxt_height_talltall[0]); // set txt ht
	oled1_putstring (&ClrTextArea[0]);    //

	// Add line below WAIT...
	oled2_send_command (&oled_drw_line_hi2[0]);
	oled1_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_height[0]);
	oled3_send_command (&oled_setxt_height[0]);

	oled1_send_command (&oled_setxt_width[0]);
	oled2_send_command (&oled_setxt_width_wide4[0]);
	oled3_send_command (&oled_setxt_width[0]);

	oled1_setxt_position (6,20);
	oled2_setxt_position (18,12);  //position for BIG WAIT.
	oled3_setxt_position (8,20);

	oled1_putstring (&Powering[0]);
	oled2_putstring (&Wait1[0]);      // put WAIT on OLED LCD
	oled3_putstring (&Fasten[0]);

	oled2_send_command (&oled_setxt_width[0]);
	oled1_setxt_position (10,70);
	oled2_setxt_position (5,56);	// set position
	oled3_setxt_position (11,70);

	oled1_putstring (&Vehicle[0]);
	oled2_putstring (&DontPress[0]);	//
	oled3_putstring (&SeatBelts[0]);

	oled2_setxt_position (5,90);	// set position
	oled2_putstring (&Accel[0]);	//
}



// --------------------------------------------------------------
// IGNITION  STATE  MESSAGES....
// Description: Loads OLEDs with
//       "Fasten"         "WAIT"        "Powering"
//     "Seat Belts"		Don't Press     "Vehicle"
//       			   "Accelerator"
// --------------------------------------------------------------
void ignition_state_screen_loads (void)
{
	// Clear the end of the "accelerator" strg
	oled1_setxt_position (15,10);   // was 10,65   Clr start position
	oled1_send_command (&oled_setxt_height_talltall[0]); // set txt ht
	oled1_putstring (&ClrTextArea[0]);    //

	// Add line below WAIT...
	oled2_send_command (&oled_drw_line_hi2[0]);

	oled1_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_height[0]);
	oled3_send_command (&oled_setxt_height[0]);

	oled1_send_command (&oled_setxt_width[0]);
	oled2_send_command (&oled_setxt_width_wide4[0]);
	oled3_send_command (&oled_setxt_width[0]);

	oled1_setxt_position (8,20);
	oled2_setxt_position (18,12);  //position for BIG WAIT.
	oled3_setxt_position (6,20);

	oled1_putstring (&Fasten[0]);
	oled2_putstring (&Wait1[0]);      // put WAIT on OLED LCD
	oled3_putstring (&Powering[0]);

	oled2_send_command (&oled_setxt_width[0]);

	oled1_setxt_position (11,70);
	oled2_setxt_position (5,56);	// set position
	oled3_setxt_position (10,70);

	oled1_putstring (&SeatBelts[0]);
	oled2_putstring (&DontPress[0]);	//
	oled3_putstring (&Vehicle[0]);

	oled2_setxt_position (5,90);	// set position
	oled2_putstring (&Accel[0]);	//
	}


// --------------------------------------------------------------
// RELOAD_BIT_WAIT_MID_OLED
// Description:
//    Run after "wait_dont_press_mid_oled" is run,to reload
//    the big  "W A I T", after clearing (Repeats)
// --------------------------------------------------------------
void reload_big_wait_mid_oled (void)
{
	//Load LARGE Wait - top part of message
	oled2_setxt_position (18,12);  //position for BIG WAIT.
	oled2_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_width_wide4[0]);
	oled2_putstring (&Wait1[0]);      // put WAIT on OLED LCD
}





 
/* ===  E V I M   S T A T E ==============================================
/ ***********************  S C R E E N    ********************************
/ ************************************  S E T U P  ***********************
/ ================================================  R O U T I N E S  ****/


/***********************************************************************
* void motor_tasks (void)
***********************************************************************/
void motor_tasks (void)  {
	// Set position for MOTOR title and output the title
	oled3_send_command (&oled_setxt_height[0]);
   	oled3_send_command (&oled_setxt_width[0]);
	oled3_setxt_position (11,20);
	oled3_putstring(&Motor[0]);
   }      

/***********************************************************************
* controller_tasks (void)
***********************************************************************/
void controller_tasks (void)
   {
     // Set position for CONTROLLER title and output the title
	oled3_send_command (&oled_setxt_height[0]);
 	oled3_send_command (&oled_setxt_width[0]);
	oled3_setxt_position (5,20);   // was 12,20
	/////oled3_putstring (&Controller[0]);
	oled3_putstring(&Control[0]);
   } 

/***********************************************************************
* dcdc_tasks (void)
***********************************************************************/
void dcdc_tasks (void)
   {
	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
     oled3_setxt_position (11,20);
     oled3_putstring(&DCDC[0]);
   } 

/***********************************************************************
* bbox1_tasks (void)
***********************************************************************/
void bbox1_tasks (void)
   {
     // Set position for B-BOX1 title and output the title
	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
     oled3_setxt_position (11,20);
     oled3_putstring(&BBox1[0]);
   } 

/***********************************************************************
* bbox2_tasks (void)
***********************************************************************/
void bbox2_tasks (void)
   {
	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
     oled3_setxt_position (11,20);
     oled3_putstring (&BBox2[0]);
   } 

/***********************************************************************
* AMBIENT_tasks (void)
***********************************************************************/
void ambient_tasks (void)
   {
     // Set position for AMBIENT title and output the title
	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
	 oled3_setxt_position (14,20);
     oled3_putstring (&Ambient[0]);
   }    

/***********************************************************************
* BATTERY_tasks (void)
***********************************************************************/
void battery_tasks (void)
   {
     // Set position for "Battery" title and output the title
	 oled1_send_command (&oled_setxt_height[0]);
 	 oled1_send_command (&oled_setxt_width[0]);
     oled1_setxt_position (15,20);
     oled1_putstring (&Battery[0]);
   }    

/***********************************************************************
* TEMP_HIGH_TASKS (void)
***********************************************************************/
void temp_high_tasks (void)
   {
 	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
     oled3_setxt_position (8,78);  // was (15,78), and then 10,78
     oled3_putstring (&High[0]);
   }    

/***********************************************************************
* TEMP_LOW_TASKS (void)
***********************************************************************/
void temp_low_tasks (void)
   {
     // Set position for LOW result
	 oled3_send_command (&oled_setxt_height[0]);
 	 oled3_send_command (&oled_setxt_width[0]);
     oled3_setxt_position (8,78);    // was 10,78
     oled3_putstring (&Low[0]);
   }    



// P E D A L   L O C K I N G   M E C H A N I S M   R E L A T E D
// P E D A L   L O C K I N G   M E C H A N I S M   R E L A T E D
// P E D A L   L O C K I N G   M E C H A N I S M   R E L A T E D 

/***********************************************************************
   Accelerator LOCKING and UNLOCKNG routines
/ **********************************************************************
/  INTERFACE BITS:
/    PORTA Bit7 = OUTPUT = Servo Control Pulse (SVO-CTRL)
/                        = 0 when idle
/    PORTA Bit6 = OUTPUT = Servo SSR PWR Enable (SVO-SSR)
/                        = 1 when enabled
/    PORTA Bit 5 = INPUT = Servo Arm Position (SRV-POS)
/                        = closed/ground when retracted/unlocked
/ --------------------------------------------------------------------------
/
/    UNLOCKED (down) Position
/        _______
/    ---/  400uS \_____________________/     
/       |<----------  5mS  ----------->|
/
/
/    LOCKED (up)Position
/        ___________________
/    ---/           1750uS   \__________/
/
/  NOTE: For low noise, using 25uS steps, and a "changing" PW, for slowly 
/        changing position (Reduces gear lash noise)
/              
/        So, for example... 1750-400/25 = 54 steps up or down
/                         (start at 16 .. 70 = 54 steps * 25uS per step)
*/        

void lock_accel_pedal_slo(void) // Unlocked=400 to Locked=1750 STEPDED
{
	//Verify pedal lock servo power (SSR) is ON  == 1
	pedal_lock_pwr_on();

    // IMPORTANT...  DO NOT REMOVE
	// ******************************
	_delay_ms(650);     // NEEDED!!  09222021
	 
	// Get current servo position (if reed sw = 0, unlocked!!)
	PORTA_image = PORTA.IN;
	if ((PORTA_image & PIN5_bm) == 0) { // So currently UNLOCKED
		//Rotate UP to locked position [400uS to 1750 uS, in steps]
		for (i=14; i< 71; i++)  {
			dly_val = (uint16_t) (25 * i); // 16*25=400uS (Strting point)
			PORTA.OUTSET = PIN7_bm;  //Start of pulse
			_delay_us(dly_val);
			PORTA.OUTCLR = PIN7_bm;  //end of 1st/nxt pulse
			if (i<20) {
				_delay_us (70000);
				}
			else if (i<28) {
				_delay_us (65000);
				}
			else if (i<36) {
				_delay_us (60000); 	}
			else if (i<42) {
				_delay_us (55000); 	}
			else if (i<50) {
				_delay_us (50000); 	}
			else if (i<60) {
				_delay_us (45000); 	}
			else {
				_delay_us (50000); 	}
			}	
		_delay_ms(10);
	}  // NOTE: Power to SERVO is ON for exit.
}

// From Locked 1750uS to unlocked 400uS (25uS steps)
void unlock_accel_pedal_slo(void)  
{

	//Verify servo power is ON,  SVO-SSR == 1
	pedal_lock_pwr_on();
		
	// Long Delay    DEFINITELY NEEEDED!!
	_delay_ms(650);     // 09222021

	// Check present position...	
	PORTA_image = PORTA.IN;
	if ((PORTA_image & PIN5_bm) != 0) {  // Then it is locked
		//Rotate DOWN = Pedal Released
		for (i=14; i<71; i++)  {
			dly_val = (uint16_t) (25 * (84-i));
			PORTA.OUTSET = PIN7_bm;  //Start of pulse
			_delay_us(dly_val);
			PORTA.OUTCLR = PIN7_bm;  //END of fist/next pulse
			      //_delay_us ((uint16_t) (64000-dly_val));  
			if (i<20) {
				_delay_us (70000); 	}
			else if (i<25) {
				_delay_us (65000); 	}
			else if (i<30) {
				_delay_us (58000); 	}
			else if (i<40) {
				_delay_us (50000); 	}
			else if (i<50) {
				_delay_us (40000);	}
			else if (i<60) {
				_delay_us (45000); 	}
			else {
				_delay_us (50000);  }
			}
		_delay_ms(30);	
	}	
	
}


/***********************************************************
/ Pedal locking mechanism power ON routine
/
/ PA6 = SSR Enable input
/ PA6 = 1 is ON; PA6 = 0 is OFF;
/
************************************************************/
void pedal_lock_pwr_on(void)
{
// PA6 = SSR for servo pedal lock
	// PA6 = 1 is ON; PA6 = 0 is OFF;
	PORTA.OUTSET = PIN6_bm;
}


/***********************************************************
/ Pedal locking mechanism power OFF routine
/
/ PA6 = SSR Enable input
/ PA6 = 1 is ON; PA6 = 0 is OFF;
/
************************************************************/
void pedal_lock_pwr_off(void)
{
	// PA6 = SSR for servo pedal lock
	// PA6 = 1 is ON; PA6 = 0 is OFF;
	PORTA.OUTCLR = PIN6_bm;
}



/***********************************************************
/ SOCH Power Down routine
/ (soch_pwr_off)
/  
/ PF3 = SSR Enable input
/ PF3 = 1 is ON; 0 is OFF;
/
************************************************************/
void soch_pwr_off (void)
{
	// PF3 = SSR for SOCH 12V-HV enable
	// PF3 = 1 is ON; PA3 = 0 is OFF;
	PORTF.OUTCLR = PIN3_bm;  // CLR == 0VDC (12V-HV is off);
}


/***********************************************************
/ SOCH Power Up routine
/ (soch_pwr_on)
/
/ PF3 = SSR Enable input
/ PF3 = 1 is ON; 0 is OFF;
/
************************************************************/
void soch_pwr_on (void)
{
	// PF3 = SSR for SOCH 12V-HV enable
	// PF3 = 1 is ON; PA3 = 0 is OFF;
	PORTF.OUTSET = PIN3_bm; 
}



//***************************************************************************
// TCA0_init 
// -----------------
// DESCRIPTION: This code starts a timer counter that if still running 
//       in 15 minutes, will timeout and shutdown the OLEDs, and return 
//       to the STANDBY state.
//
//  8 MHz clock divided by 1024 == 7812.5 Hz
//
//  1 / 7812.5 == 0.0001 == 100 uS (per timer tick!)
//
//  Overflow INTR is at 64K * 100uS == 6.5535 seconds
//
//  15 Mins / 6.5535 secs == 137.33 
//            (= # of ISR triggers before retirning to STANDBY_STATE)

//
//**************************************************************************
void TCA0_init(void)
{
 /* enable overflow interrupt */
 TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
 /* set Normal mode */
 TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
 /* disable event counting */
 TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTAEI_bm); 
  
 /* set the period */
 TCA0.SINGLE.PER = 65535;  // Full 16 bit 64K range
 
 /* set clock source */
            //TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc 
 TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc
                    | TCA_SINGLE_ENABLE_bm; /* start timer */
}



//***************************************************************************
// TCA0_stop
// -----------------
// DESCRIPTION: This code starts a timer counter that if still running 
//       in 15 minutes, will timeout and shutdown the OLEDs, and return 
//       to the STANDBY state.
//
//  8 MHz clock divided by 1024 == 7812.5 Hz
//
//  1 / 7812.5 == 0.0001 == 100 uS (per timer tick!)
//
//  Overflow INTR is at 64K * 100uS == 6.5535 seconds
//
//  15 Mins / 6.5535 secs == 137.33 
//            (= # of ISR triggers before retirning to STANDBY_STATE)
//
//**************************************************************************
void TCA0_stop (void)
{
  // set CTRLA to stop timer counter unit
 // (Set to reset value) 
 TCA0.SINGLE.CTRLA = 0;

 /* disable overflow interrupt */
 TCA0.SINGLE.INTCTRL = 0;
 
 timeout_counter = 0; // added 12/16
}



////////////////////////////////////////
////////////////////////////////////////
////    ISR Include Routines
////  ----------------------------------
# include <ISR.h>
#include <ISR_Titles_SwitchCase_Quadrature_Release3.inc>
////
////////////////////////////////////////



//===============================================================================
//  Voltage converting channels are:
//		1. ADC7 = PD7 = V(batt13)  [X 0.2732 scale down factor, assuming 15V Max]
//		2. ADC6 = PD6 = V(Aux12V)  [X 0.2732 scale down factor, assuming 15V Max]
//		3. ADC18 = PF2 = V(Aux5V)  [X 0.75 scale down factor, assuming 5.5V Max]
//
//	All voltage conversions use the 4.096V reference!
//===============================================================================


//****************************************************************
// load_133V_battery_voltage (void)                    AVR128DA48
//
// Description:	This routine converts the voltage on ADC7 (PD7),
//				which is the accy lithium battery voltage and 
//				loads global variables for 13.3V voltage value 
//****************************************************************
void load_133V_battery_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
    uint16_t tens;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN7_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD7 = ADC7
	ADC0_MUXPOS = 0x07;  // Selects channel6, which is accy batt voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	  // Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	  // ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
							 
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low) +30);

	multiplier = (uint16_t) 1000;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	current_batt_voltage = (product / ((uint16_t) 272));

	// -------------------------------------------
	// OFFSET/Correction Adjustment...  
	// current_batt_voltage = current_batt_voltage+150;  // ~ 0.15 volt increase!
	// 12/16/22 current_batt_voltage = current_batt_voltage - 80;
	current_batt_voltage = current_batt_voltage - 110;  // Due to in car shift?!	
	// -------------------------------------------
	
	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1
	
	// Convert to BCD digits for display (process & separate
	// digits), and load global variables
	current_batt_voltage = current_batt_voltage/10; // reduce mv to TENTHS of mV
	hundredths = current_batt_voltage%10;
	accy133_hundredths = hundredths;
	   
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;
	accy133_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	accy133_units = units;
	
	current_batt_voltage = current_batt_voltage/10;
	tens = current_batt_voltage;
	accy133_tens = tens;
			    
}
		


//****************************************************************
// void load_a12V_voltage (void) 
//
// Description:	This routine converts the HV auxillary 12V supply on
//				ADC6 (PD6), which is the aux HV-to-12V SOCH and SDT
//				voltage, and stores the result digits in the 
//              associated global variables.
//****************************************************************
void load_a12V_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
    uint16_t tens;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN6_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD6 = ADC6
	ADC0_MUXPOS = 0x06;  // Selects channe6, which is aux 12V voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	// Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	// ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low) +30);

	multiplier = (uint16_t) 1000;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	current_batt_voltage = (product / ((uint16_t) 272));

	//////// OFFSET/Correction Adjustment...  
	//////// OFFSET/Correction Adjustment...  
	current_batt_voltage = current_batt_voltage + 200; // ~ 0.200 volts added
	//////// OFFSET/Correction Adjustment...  
	//////// OFFSET/Correction Adjustment...

	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1

	// OPTIONAL NOISE REDUCING CODE...
    //// current_batt_voltage = current_batt_voltage >> 3;  // for ACCUM reduction
	
	//Convert to BCD digits for display 
	current_batt_voltage = current_batt_voltage/10; // reduce mvs to 1/10s of mV
	hundredths = current_batt_voltage%10;   // process & separate digits
	aux12_hundredths = hundredths;
	
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;
	aux12_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	aux12_units = units;
	
	current_batt_voltage = current_batt_voltage/10;
	tens = current_batt_voltage;
	aux12_tens = tens;
}





//********************************************************************
// load_a5V_voltage (void)                            AVR128DA48
//
// Description:	This routine converts the voltage on ADC18 (ADC0.18),
//              Pin PF2, which is the HV based 5V auxillary voltage,
//				and stores the value digits in the associated global
//              variables 
//********************************************************************
void load_a5V_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN6_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD6 = ADC6
	ADC0_MUXPOS = 0x12;  // Selects channe6, which is aux 12V voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	// Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	// ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low));  
													// correction if needed +30);
	multiplier = (uint16_t) 1467;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	//current_batt_voltage = (product / ((uint16_t) 272));
	current_batt_voltage =   (uint16_t) (product / ((uint32_t) 1000));

	// -----------------------------------------------------------------
	//OFFSET/Correction Adjustment...  
	//    current_batt_voltage = current_batt_voltage + 50; // ~ 0.05 volts added!
	current_batt_voltage = current_batt_voltage; // ~ 0.00 volts added!
	// -----------------------------------------------------------------
	
	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1

	//Convert to BCD digits for display 
	current_batt_voltage = current_batt_voltage/10; // reduce mvs to tenths of mV
	hundredths = current_batt_voltage%10;   // process & separate digits
	aux5_hundredths = hundredths;
	
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;  
	aux5_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	aux5_units = units;

}
		
////////////////////////////////////////
////////////////////////////////////////
////    ESP32 ISR Include Routines
////  ----------------------------------
# include <ESP32_ISR.h>
# include <ESP32_ISR_Remote_InterfaceRoutines.inc>
////
////////////////////////////////////////



//////////////////////////////////////////////////////////////////
////////     M A I N    E N T R Y   P O I N T      ///////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
////////     M A I N    E N T R Y   P O I N T      ///////////////
//////////////////////////////////////////////////////////////////

 int16_t main (void) {

    // =====================================	
	//Entry Point for Main (Cold Reset)	
    // =====================================
	
	// SET SYSTEM CLOCK TO 8MHz !!!!
	fcpu_init();

	uint32_t i = 0;   // ctr 4 determining when to STROBE 
	uint32_t i_soch = 0;   // ctr 4 determining when to access SoCH 
		
	uint32_t flash_count = 0;   // counter for determining when to STROBE 
								// the Alarm-LED When in STANDBY lower loop								
uint8_t PORTA_IN_image = 0;	  // PortA image at the occurrence of intr
uint8_t PORTB_IN_image = 0;	  // PortB image at the occurrence of intr
uint8_t PORTC_IN_image = 0;	  // PortC image at the occurrence of intr
uint8_t PORTE_image = 0;      // PortE image at the occurrence of intr

	uint8_t new_tail = 0;

	// MOVED UP 1/11/2023
	/* PORTA DDR INIT CODE   (Note: PA0 & PA1 init in USARTs_Init subr)
	/  -------------------------------------------------------------------
	/  PA2=OLEDRST (Act-Low); PA3=PWR_DEV (Act-Low); PA4=CRG5 (Act-hi) */
	PORTA.DIRSET = (PIN2_bm | PIN3_bm | PIN6_bm | PIN7_bm );  //all OUTPUT
	PORTA.DIRCLR = (PIN4_bm | PIN5_bm);  // all these bits are INPUT

	// INIT PowerDown of all LEDS, periph devices and SSRs...
	PORTA_OUTSET = PIN3_bm;	// PWRDEV = '1' = ALL Devices (OLEDs+) to OFF.
	PORTA_OUTCLR = PIN2_bm;	// Set RESET to '0 FOR LOW POWER...

	// Clear the SRVO control input (=0) and power-up 
	// servo (for holding in LOCK pos)
	PORTA_OUTCLR = PIN7_bm;	//  = Servo PWM control signal == 0


	// PB5 = Sounder = OUTPUT == Active High (Set OFF = 0)
	PORTB.DIRSET = PIN5_bm;  // Sounder output drive bit
	PORTB.OUTCLR = PIN5_bm;  

	// POWERUP DOUBLE BEEP-TONE... AT COLD START RESET
	tone();
    _delay_ms(175);
	tone();

	// CAREFULLY VERIFY....
	//
	// NOT NEEDED ....  REMOVE??
	// NOT NEEDED ....  REMOVE??
	// NOT NEEDED ....  REMOVE??

	// INIT ALL VARIABLES!
	PORTA_IN_image = 0;		// PortA image at the occurrence of intr
	PORTB_IN_image = 0;		// PortB image at the occurrence of intr
	portB_image_old = 0;
	PORTC_IN_image = 0;		// PortC image at the occurrence of intr
	PORTE_image = 0;        // PortE image at the occurrence of intr
	new_tail = 0;

	state_num = 4;    // Start with Controller temp view
	top_state_num = 0; // Standby State (Low Power w/OLEDs off)
	temp_port_image = 0;

		// DEBUG CODE LEVEL == 3...   NORMAL = 1...
		contrast_level = 1;  // DEBUG=3: Normal value = 1 = lowest power;
		contrast_oldval = 1;  
		// DEBUG CODE LEVEL == 3...   NORMAL = 1...

	first_time = 0;
	serial_command_byte = 0;   
	charge_cycle_active_flag = 0; 
	warm_restart_flag = 0;;

	dsp_mode_flag = 0;     //  
	dsp_mode_sw_val = 0;   //  current value of MODE-SW
	direction_flag=0;      //  0 = CCW, otherwise CW...
	current_state = 0;     //  MOTOR_STATE;
	receive_word = 0;     

	//OLED BUSY & NAK FLAGS - Version 7 Addition for || processing 
	oled1_busy_flag = 0;  // When a '1', must wait for executing
	oled2_busy_flag = 0;  // command to end (which will reset
	oled3_busy_flag = 0;  // the flag back to '0'...
	
	
	oled1_NAK_flag = 0;   // When a '1', must wait for executing
	oled2_NAK_flag = 0;   // command to end (which will reset
	oled3_NAK_flag = 0;   // the flag back to '0'...

	// Required STATE status flags...
	standby_state_active_flag = 1;
	evim_state_active_flag = 0;
	tailite_flag = 0;

	count = 0;    // DEBUG VAR
	first_response_char = 0;

	////  ISR RELATED VARIABLES
	PORTA_image = 0;        // PortD image at the occurrence of intr
	portB_image = 0;
	PORTC_image = 0;        // PortC image at the occurrence of intr
	PORTE_image = 0;        // PortE image at the occurrence of intr
	new_dsp_mode_sw_val = 0;

	ambient_active = 0;   // flag for ambient state active
	interrupted_state = 0;     // holds state to return






//	/* PORTA DDR INIT CODE   (Note: PA0 & PA1 init in USARTs_Init subr)
//	/  -------------------------------------------------------------------
//	/  PA2=OLEDRST (Act-Low); PA3=PWR_DEV (Act-Low); PA4=CRG5 (Act-hi) */
//	PORTA.DIRSET = (PIN2_bm | PIN3_bm | PIN6_bm | PIN7_bm );  //all OUTPUT
//	PORTA.DIRCLR = (PIN4_bm | PIN5_bm);  // all these bits are INPUT
//
//	// INIT PowerDown of all LEDS, periph devices and SSRs...
//	PORTA_OUTSET = PIN3_bm;	// PWRDEV = '1' = ALL Devices (OLEDs+) to OFF.
//	PORTA_OUTCLR = PIN2_bm;	// Set RESET to '0 FOR LOW POWER...
//
//	// Clear the SRVO control input (=0) and power-up 
//	// servo (for holding in LOCK pos)
//	PORTA_OUTCLR = PIN7_bm;	//  = Servo PWM control signal == 0







	pedal_lock_pwr_off();    //#1 after main entry

	PORTA_PIN5CTRL |= PORT_PULLUPEN_bm;  // Enable pullup for servo reed-sw
		    								 // 0 == unlocked position
	// Enable the CHARGE input interrupt
	PORTA.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc;  // CHARGE Signal Interrupt
												
	// PORTB DDR INIT CODE   (Note: PB0 & PB1 init in USARTs_Init subr)
	// ------------------------------------------------------------------
	//   PB2= RPG-CHA; PB3= RPG-CHB; PB4= RPG-PBSW(NO);
	//   Notes: PB5 = Sounder output is DONE ABOVE 
	//          No PULLUPS NEEDED for PORTB
	PORTB.DIRCLR = (PIN2_bm | PIN3_bm | PIN4_bm);  // All inputs

	// PORTC DIR SET INIT CODE  (Note: PC0 & PC1 init in USARTs_Init subr)
	// -------------------------------------------------------------------
	// PC2= F/C; PC3= PWRSW; PC4= OUTPUT LED Flash; 
	// PC5= Tailite; PC6= Accy; PC7= IGN
	PORTC.DIRCLR = (PIN2_bm|PIN3_bm|PIN5_bm|PIN6_bm|PIN7_bm);
	PORTC.DIRSET = (PIN4_bm);  // Alarm LED output bit (for STANDBY)
		
	// Enable Pull-up for SPST F/C switch == PC2	
	PORTC_PIN2CTRL |= PORT_PULLUPEN_bm;  // NO Intr for this switch  

	// Enable Pull-up and Intr for SPST PWRSW mode-select sw (PC3)
	PORTC_PIN3CTRL |= (PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc); 
	

    // PORTD Analog ADC INPUT CONFIGURTION 
    //-------------------------------------------------------------------------
    // PORTD & PF2 = Analog inputs w/basic setup initialization 
    // Analog Input Definitions:
    // ADC0.0  PD0= Motor NCT10K sensor (Warp11 Netgen DC Series Motor)
    // ADC0.1  PD1= DC HV Controller NCT10K sensor (Raptor 1200 controller)
    // ADC0.2  PD2= DC-DC Converter Heat Sink Plate NTC sensor
    // ADC0.3  PD3= Battery Box #1 NCT10K sensor (midcar) 
    // ADC0.4  PD4= Battery Box #2 NCT10K sensor (trunk)
    // ADC0.5  PD5= Ambient NCT10K sensor (frnt drvr-side motor compartmnt)
    // ADC0.6  PD6= V(HVAux-12V) voltage input [15V max]
    // ADC0.7  PD7= V(Accy13) voltage input [15V max]
    // ADC0.18 PF2= V(Aux5V) voltage input [6V max]

	PORTD_DIRCLR = 0xff;  // set all to inputs  REQ'D for ADC use ????????
	
	// Disable all input buffers... PIN CTRL ISC bits == "100" = Disabled
	PORTD.PIN0CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;  
	PORTD.PIN1CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN2CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN3CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN4CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN4CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN5CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN5CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN6CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc; 
	PORTD.PIN7CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
	PORTD.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc; 

	// Verify all pullup-Rs are DISABLED 
	// (PORT_PULLUPEN_bm = 0x08 (Bit3 = Pullup enable bit)
	PORTD.PIN0CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN2CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN3CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN4CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN5CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)
	PORTD.PIN7CTRL &= ~PORT_PULLUPEN_bm;  // clr Pullup Enable bit (Bit3)

	// PORTE DIR SET & INIT CODE (Only 4-bits: PE3-PE0)
	// ------------------------------------------------------------------
	//   Bit Definitions:
	//       PE3 = Seat (Pullup Req'd)
	//       PE2 = Keyin (Pullup Req'd)
	//       PE1 = Door (Pullup Req'd)
	//		 PE0 = Cntctr
	PORTE.DIRCLR = (PIN3_bm | PIN2_bm | PIN1_bm | PIN0_bm);  // INPUTS...
	PORTE.PIN3CTRL |= PORT_PULLUPEN_bm;  // Seat PullUp: Msk=0x08 =Set bit3
	PORTE.PIN2CTRL |= PORT_PULLUPEN_bm;  // Keyin PullUp: All Pull-Ups ON
	PORTE.PIN1CTRL |= PORT_PULLUPEN_bm;  // Door Sw PullUp
		
	// PORTF DIR SET INIT CODE
	// --------------------------------------------------------------------
	// Init for VCC4 signal (used to measure system aux 5VDC level
	PORTF.PIN2CTRL &= ~PORT_ISC_gm;   // clear Input sense bits (0x07)
		
	// Digital Input Buf disable (0x04<<0) == Bit2=1
	PORTF.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;  
	PORTF.PIN2CTRL &= ~PORT_PULLUPEN_bm;   // clr Pullup Enable bit (Bit3)

	PORTF.DIRSET = PIN3_bm; // Set HV SSR+ signal == output
	PORTF.OUTCLR = PIN3_bm; // CLR == HV 12VDC PS is OFF...
   					// (So both SoCH and SDT OFF!)

	// Configure RESET input (PF6)  [DEF=reset pin]
	// Set on RESET in RSTCTRL.RSTFR
	// BUT input pullup must be enabled.
	PORTF.PIN6CTRL = PORT_PULLUPEN_bm;

	// PRE-SCALER INIT...
	ADC0_CTRLC = ADC_PRESC_DIV2_gc; // CTRLC = prescaler BITS only

	// -------------------------------------------------------------------
	// E N D   OF   P O W E R - U P    I N I T    C O D E
	// E N D   OF   P O W E R - U P    I N I T    C O D E
	// -------------------------------------------------------------------

 
 
 

// "TOP LEVEL STATES" OVERALL WHILE-ONE LOOP
while (1)
{
	// S T A N D B Y   S T A T E   T O P   L E V E L   E N T R Y
	// S T A N D B Y   S T A T E   T O P   L E V E L   E N T R Y
	// S T A N D B Y   S T A T E   T O P   L E V E L   E N T R Y
	// =======================================
	while (top_state_num == STANDBY_STATE) {

		// ENABLE REQUIRED INTERRUPTS (For MinVer only using IGN and CHRG)
		// --------------------------
		// PA4=CHRG Signal :: PC7=IGN Signal :: Enable both edges of each
		PORTA.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc; // CHRG intr ON
		PORTE.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc; // KEYIN Intr On
		PORTE.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc; // Door ON 
									// (For energizing ped-loc mech)

		// DISABLE UNNEEDED INTERRUPTS (Assume nothing!)
		// ---------------------------------------------------------
		// PORTB : RPG : DISABLE : Not Needed
		PORTB.PIN4CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // RPG PBSW (NO)
		PORTB.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // RPG CH-B
		PORTB.PIN2CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // RPG CH-A

		// PORTC : DISABLE : Not Needed
		PORTC.PIN7CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Ign signal OFF 
		PORTC.PIN6CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Accy5 signal
		PORTC.PIN5CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Tailite5 signal
		PORTC.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Pwr-mode Switch 
		PORTC.PIN2CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // F/C select switch

		PORTE.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // SEAT OFF
		PORTE.PIN0CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // Cntctr
		cli ();

		// TEST FOR WARM RESET... MUST Check IGN=12V *OR* CHRG Input Signal
		PORTA_IN_image = PORTA.IN;  //PA4 == Charge Mode Signal
		PORTC_IN_image = PORTC.IN;  //PC7 == IGN, PC6=Accy, PC5=tail
			
		// Power down SOC head and SDT module
		soch_pwr_off();
	
		//12/15
		rpg_on_flag = 0;


 		// WARM RESTART CHECK  /////0507
		// If IGN = 12V and CTCR closed, enter EVIM MODE
		if (((PORTC_IN_image & PIN7_bm) != 0) && (PORTE.IN & PIN0_bm)!=0) { 
			top_state_num = WAKE1_STATE;      // If so, go to...
			standby_state_active_flag = 0;    //    WAKE1 Pseudo STATE 
 			charge_cycle_active_flag = 0;  // NOT a charge cycle
			evim_state_active_flag = 1;    // restart/warm reset
			warm_restart_flag = 1;         // ('warm' due to IGN=12V, +)

			// SAFETY ONLY *** SAFETY ONLY *** SAFETY ONLY *** SAFETY ONLY
  			// UNLOCKED Accelerator Pedal? (If NOT, move to UNLOCK *DOWN* position)
			if (rpg_on_flag==0) {  
			  unlock_accel_pedal_slo();  // 09220221
			}
		}
			
	    // TEST for in Charge MODE... 
	    else if ((PORTA_IN_image & PIN4_bm) != 0) { //OK, Charge Sig active
		    top_state_num = WAKE1_STATE;
			standby_state_active_flag = 0;
		    evim_state_active_flag = 0; 
		    charge_cycle_active_flag = 1; // charge cycle
			warm_restart_flag = 0;
			
			// *VERIFY* LOCKED Accelerator Pedal in *UP* position
			lock_accel_pedal_slo();   //
		}
		else {  // Going into STANDBY
			top_state_num = STANDBY_STATE;
			standby_state_active_flag = 1;
			charge_cycle_active_flag = 0; // NO charge cycle
			evim_state_active_flag = 0;   //   and no IGN state
			warm_restart_flag = 0;

			// *VERIFY* LOCKED Accelerator Pedal in *UP* position 
			lock_accel_pedal_slo();
		}
			
		//  IS THIS NEEDED FOR RE-ENTRY / RESET?
		PORTA_OUTSET = PIN3_bm;  // '1' == PWR DOWN OLED, RPG, plus...
		PORTA_OUTCLR = PIN2_bm;  // '0' == reset OLEDs, for LOW POWER here!
		sei();
	
	
		// I M P O R T A N T  - - - - - - - - - - - - - - - 
		// Returns system to "powerup" conditions...
		VREF_ADC0REF = 0;
		ADC0_CTRLA = 0; //reset all bits...	
		USART0.BAUD = 0;
		USART1.BAUD = 0;
		USART2.BAUD = 0;
		USART3.BAUD = 0;
	 
		// Disable all four USARTs
		USART0.CTRLB = 0;
		USART1.CTRLB = 0;
		USART2.CTRLB = 0;
		USART3.CTRLB = 0;
		
	
		// L O W E R  STANDBY   W A I T   L O O P (w\Strobing "Alarm Armed" LED) 
		// L O W E R  STANDBY   W A I T   L O O P (w\Strobing "Alarm Armed" LED) 
		// L O W E R  STANDBY   W A I T   L O O P (w\Strobing "Alarm Armed" LED) 
		while (top_state_num == STANDBY_STATE)
		  {
			// TEST flags for Charge MODE or EVIM MODE... Enter EVIM_STATE
			if ((charge_cycle_active_flag == 1) | (evim_state_active_flag == 1)) {
				top_state_num = WAKE1_STATE;  // On the way to EVIM!
				standby_state_active_flag = 0;
			}

			// STROBE GENERATING CODE SECTION
			// Conditions:
			// Door closed == 1 (PE1 = 1)
			// Seat empty == 1 (PE3 = 1)
			// Keyout == 0  (PE2 = 0)

			// -------------------------------------------------------
			//          KEYOUT                         DOOR Open
			// -------------------------------------------------------
			if (((PORTE.IN & PIN2_bm) == 0)  && ((PORTE.IN & PIN1_bm) != 0))  {
				pedal_lock_pwr_off();
				flash_count++;  // increment during each pass
			
				// TEST if flash_count has reached the "strobe" value..
				if (flash_count >= stobe_cnt_value) { // If =, then Strobe LED...
					// Turn on LED (PC4 = '1')
					PORTC.OUTSET = PIN4_bm;		// active high LED
					//STB generation delay...  (INTERRUPTABLE)
					for (i=0; i<300; i++) {  // INCREASE to increase STB length
					  _delay_ms(1);
								
					  // TEST flags for Charge MODE or EVIM MODE
					  if ((charge_cycle_active_flag==1)|(evim_state_active_flag==1)) {
						top_state_num = WAKE1_STATE;  // On the way to EVIM!
						standby_state_active_flag = 0;
						flash_count = 127;
					  }				
					}

					// Turn OFF LED (PC4 = '0')
					PORTC.OUTCLR = PIN4_bm;		// Now off... wLow Level
					flash_count = 0;
				}
			}  // If for DOOR and SEAT
			_delay_ms(1);

	} // End of LOWER STANDBY Loop
} // End of STANDBY



	// ===============================================================
	//  P S E U D O   W A K E 1   S T A T E   E N T R Y
	//  P S E U D O   W A K E 1   S T A T E   E N T R Y
	// ===============================================================
    //  ONE PASS ONLY...  Powers up the OLED modules...  This is
	//  a pseudo or pass-thru state on the way to the EVIM_STATE!
	//
	while (top_state_num == WAKE1_STATE) {  
		// Setup Interrupts for this state
		cli();
	
		soch_pwr_on();  // Turn on HV to 12VDC module
		
		// USART INITIALIZATION - All Four!
		//   USART0 = OLED1 (Left)
		//   USART1 = OLED3 (Right)
		//   USART2 = SOC Head
		//   USART3 = OLED2 (Middle) */
		USARTs_Init ();

		// DISABLE UNNEEDED INTERRUPTS (assume nothing at this time!!!!)
		// --------------------------------------------------------------
		PORTB.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // RPG CH-B
		PORTB.PIN2CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // RPG CH-A
		PORTC.PIN6CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Accy5 signal
		PORTC.PIN5CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Tailite5 signal
		PORTC.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Mode Switch (OLED2)
		PORTE.PIN0CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // Cntctr


		//03012022
		PORTE.PIN1CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // DOOR OFF

		// ENABLE REQUIRED INTERRUPTS
		// --------------------------------------------------------------
		PORTA.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc;  // CHRG intr ON
		PORTC.PIN7CTRL |= PORT_ISC_BOTHEDGES_gc;  // Ign5 intr ON

		////////////  Added 12/15/2022
		PORTB.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc; // RPG PBSW NO=logic-0)

		// Power DOWN (to be sure), then powerup the OLED modules...				
		PORTA_OUTSET = PIN2_bm;	// set RESET to '1' == inactive
		_delay_ms(1);   // Short delay
		PORTA_OUTSET = PIN3_bm;	// PWRDEV to '1' = ALL Devices/OLEDs OFF.
										
		_delay_ms(60);  // Power down delay
		PORTA_OUTCLR = PIN3_bm;  // '0' == powerup OLED, RPG, plus...

		_delay_ms(2);	
		oled_init();			 // Init ALL uOLED modules

		PORTC_IN_image = PORTC.IN;  //PC7 == IGN, PC6=Accy, PC5=tail
		
		// 12/15/2022		
      	//   NEEDEED... ABSOLUTELY !!!!!!
		top_state_num = EVIM_STATE;

		// Check for SoCH MANUAL/Driver Reset Requirements: 
		//		1. IGN ON (Car has been started)
		//		2. RPG knob pressed down (before reset)  == Logic-1 
		//		3. Press ext-reset PB (Under center dash)
		//
		//	This will cause a RESET of the SoCH Module:
		//		1. SoC resets to 99%
		//		2. amp/hours set to 0
		//		3. watt/hours set to 0
		//  ADDED 01212021 PM
		//
        //      if RPG-PB down (=1)   *and*    IGN=12V (=1)		
		if (((PORTB.IN & PIN4_bm) != 0) && ((PORTC.IN & PIN7_bm) != 0)) { 

			SOC_UART2_SndCmd(soc_reset);
			tone();
			_delay_ms(100);
			SOC_UART2_SndCmd(soc_reset);
			tone();
			_delay_ms(100);
			tone();
			}

		// If EVIM Mode, Run : If not, don't exec for Charge/RPG-On w/Contactor-Open 
		if ((evim_state_active_flag == 1) && (warm_restart_flag == 0)) {  		

			//Clear ALL Text Areas (leave lines!)
			oled1_setxt_position (4,6);   // Clr start position
			oled2_setxt_position (4,6);   // Clr start position
			oled3_setxt_position (4,6);   // Clr start position

			oled1_send_command (&oled_setxt_height_talltall[0]); // set txt ht
			oled2_send_command (&oled_setxt_height_talltall[0]); // set txt ht
			oled3_send_command (&oled_setxt_height_talltall[0]); // set txt ht

			oled1_putstring (&ClrTextArea[0]);    //
			oled2_putstring (&ClrTextArea[0]);    //
			oled3_putstring (&ClrTextArea[0]);    //
			
			oled1_send_command (&oled_setxt_FG_colorWHT[0]);  
			oled2_send_command (&oled_setxt_FG_colorWHT[0]);  
			oled3_send_command (&oled_setxt_FG_colorWHT[0]);  
		
  			// Load all OLEDs...
			ssc_oled_lines(); 
			entering_evim_state_screen_loads();


			// W A I T   f o r  I G N I T I O N   S i g n a l   L O O P
			// W A I T   f o r  I G N I T I O N   S i g n a l   L O O P
			// W A I T   f o r  I G N I T I O N   S i g n a l   L O O P
  			while ((PORTC.IN & PIN7_bm) == 0) {	
				cli();
				//Reset timeout for all OLEDs
				oled1_setxt_position (4,6);   // activity refresh
				oled2_setxt_position (4,6);   // (required)
				oled3_setxt_position (4,6);   // 
				sei();				




				// Wait loop for Ignition signal, which is NOT coming along
				// For WiFi Access !!
				//-----------------------------------------------------------
				//
				// Get and store the low voltage digits in global variable:
				load_a5V_voltage();
				load_a12V_voltage();
				load_133V_battery_voltage();
			
				// Load SoCH values (V, I, %SOC, kWh)	
				cli();
				verify_SOCH_online();
		
				// Get and store the low voltage digits in global variable:
				// -------------------------------------
				//Wait for SOCH to be ready...
				_delay_ms(10);    // 20 worked...
	
				if (soch_offline_flag == 0)  {    // Then online, execute ...
			        // Update SoCH values (Vpack, Ipack, SoC, kWh)
			        SOC_UART2_SndCmd (&get_pack_voltage[0]);  //
					Get_SOC_Response(&pack_voltage_array[0], 'V');
					_delay_ms(5);

					SOC_UART2_SndCmd (&get_pack_current[0]);  //
					Get_SOC_Response(&pack_current_array[0], 'A');
					_delay_ms(5);

					SOC_UART2_SndCmd (&get_pack_soc[0]);  //
					Get_SOC_Response(&pack_soc_array[0], '%');
					_delay_ms(5);

					SOC_UART2_SndCmd (&get_watthours_soc[0]);  //
					Get_SOC_Response(&pack_kwh_array[0], 'W');
					_delay_ms(5);
				}
						 

				// Get and store the current temperature "raw" values
				// (Must be processed/scaled to get digit values) 
		        // ****************************************************
			    get_temps();    // Load all current temps into array				
				
				scale_temps_array();
				




				// TEST for in Charge MODE...
				if ((PORTA.IN & PIN4_bm) != 0) { //OK, Charge Sig active
				    _PROTECTED_WRITE(RSTCTRL.SWRR, PIN0_bm);
				}
			
				//MUST Test for rpg_on_flag === 1...
				if (rpg_on_flag == 1) {
					break;	 
				}
			}  // END Of While (PORTC.IN & PIN7_bm)
		 
		 
		 
		if ((rpg_on_flag != 1) && (warm_restart_flag == 0)){

			// OK... Load next screen with large WAIT
			//-------------------------------------------------
			contrast_level = 15;   // Max value
			oled_contrast_set_cc(contrast_level);
			_delay_ms(150);

			cli();
			// clear WAIT...
			oled2_send_command (&oled_setxt_width_wide4[0]);
			oled2_setxt_position (18,12);  //position for BIG WAIT.
			oled2_putstring (&ClrWait[0]);			
			sei();

			tone();
			_delay_ms(350); ///////   WAS 300 03132022
			
			cli();
			reload_big_wait_mid_oled ();
			oled1_setxt_position (3,14);
			oled3_setxt_position (3,14);
			sei();

			// WAIT FOR FRONT CONTACTOR TO CLOSE...								
			while (((PORTE.IN & PIN0_bm)==0) && (top_state_num==EVIM_STATE)) {	
				//run each time through
				_delay_ms(150);

				cli();
				// clear WAIT...
				oled2_send_command (&oled_setxt_width_wide4[0]);
				oled2_setxt_position (18,12);  //position for BIG WAIT.
				oled2_putstring (&ClrWait[0]);			

				sei();
				tone();
				_delay_ms(400);  // REQUIRED!   was 300 03102022
			
				cli();
				reload_big_wait_mid_oled ();
				oled1_setxt_position (3,14);
				oled3_setxt_position (3,14);
				sei();

					////////////////////////////////////////////////////////////
					// ADD WITH FSM VERSION
					////////////////////////////////////////////////////////////
					// FUTURE ENHANCEMENT... ADD TIME OUT *IF* FRONT CONTACTOR
					//                       DOES NOT CLOSE ....................
					////////////////////////////////////////////////////////////
			}
		} // End of IF (rpg_on_flag != 1) && (warm_restart_flag == 0)


		// VERIFY this is NOT a warm-reset with EV powered up (IGN=1)
		//		IGN=1           *and*       NOT a warm start event
		if (((PORTC.IN & PIN7_bm) != 0) && (warm_restart_flag==0))
		    {

			// CLEAR TEXT from Middle OLED
			oled2_setxt_position (2,7);   // WAS (4,5)   Clr start position
			oled2_send_command (&oled_setxt_height_talltall[0]); // set txt height
			oled2_putstring (&ClrTextArea[0]);    //
		
			// Reset/Refresh OLED1 & OLED3 activity time-out count				
			oled1_setxt_position (3,14);
			oled3_setxt_position (3,14);

			// Reload Middle Display
			oled2_send_command (&oled_setxt_height[0]);
			oled2_send_command (&oled_setxt_width[0]);

			oled2_setxt_position (10,12);
			oled2_putstring (&Running[0]);  // Load 

			oled2_setxt_position (5,50);   
			oled2_putstring (&DIAGNOSTIC[0]);
			oled2_setxt_position (15,88);
			oled2_putstring (&Checks[0]);

			tone();

				///////	FOR DISPLAY LAYOUT VERIFICATION..				
				///////		while (1) {		
				///////			oled1_setxt_position (3,14);
				///////			oled2_setxt_position (3,14);
				///////			oled3_setxt_position (3,14);
				///////		}
		}

		if (evim_state_active_flag == 1) { // EVIM related, NOT Chg related
			///////////unlock_accel_pedal_slo();  // SAFETY RELATED - PEDAL UNLOCK

			if (rpg_on_flag==0) {  
			  
			  unlock_accel_pedal_slo();  // 09220221
			}
		}

		sei();  
		
	} // END Of... If (evim_state_active_flag == 1) && (warm_restart_flag == 0)
		
} //END Of *pseudo* WAKE1_STATE









// ============================================================================
// ============================================================================
// E V I M   S T A T E   E N T R Y   (I N S T R U M E N T A T I O N    M O D E)
// E V I M   S T A T E   E N T R Y   (I N S T R U M E N T A T I O N    M O D E)
// E V I M   S T A T E   E N T R Y   (I N S T R U M E N T A T I O N    M O D E)
// ============================================================================
// ============================================================================
  while (top_state_num == EVIM_STATE)  {

	cli();  
	
	// OK Shutdown the TIMEOUT TCA0 feature IFF Ign = 1 and rpg_on_flag = 0
	// if      IGN=0          *and*  RPG-PB pressed=0x10 
    if (((PORTC.IN & PIN7_bm) != 0) && (rpg_on_flag == 0)) {
		TCA0_stop();
		}


	// DETERMINED NOT TO BE NEEDED 12/19/2022
	// NEEDED ?????????????????????????????????? 
	//
	// Power up SOC head and SDT module
   	////////    soch_pwr_on();

	// DEBUG VALUE   DEBUG VALUE   DEBUG VALUE
	contrast_level = 1;   // DEBUG VALUE == 3 Should be 1
	contrast_oldval = 1;
	oled_contrast_set_cc(contrast_level);  // set to last setting, as

	// DISABLE UNNEEDED INTERRUPTS
	// --------------------------
	PORTC.PIN7CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Ign5 signal
	PORTC.PIN6CTRL &= ~PORT_ISC_BOTHEDGES_gc;  // Accy5 signal
	PORTE.PIN3CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // Seat SW
	PORTE.PIN0CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // Cntctr

	// ENABLE REQUIRED INTERRUPTS
	// --------------------------
	PORTA.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc;  // CHRG intr ON
	PORTC.PIN7CTRL |= PORT_ISC_BOTHEDGES_gc;  // Ign5 signal
		
	//PORTB: RPG Signals : ENABLE BOTH EDGES
	PORTB.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc;  // RPG PBSW (NO)
	PORTB.PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;  // RPG CH-B
	PORTB.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc;  // RPG CH-A

	// Tailite and SoC Display Switch (ENABLE BOTH EDGES)
	PORTC.PIN5CTRL |= PORT_ISC_BOTHEDGES_gc;  // Tail light
	PORTC.PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;  // Dsply SoC Mode Sw

	PORTE.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc;   // Keyin
	if (charge_cycle_active_flag==1) {	
		PORTE.PIN1CTRL &= ~PORT_ISC_BOTHEDGES_gc;   // Door intr disabled
		}
	else {
		PORTE.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;   // Door intr Enabled
		}

	// Clear any pending/spurious interrupts
	PORTA.INTFLAGS = 0xff;    // clear all pending!
	PORTB.INTFLAGS = 0xff;
	PORTC.INTFLAGS = 0xff;
	PORTE.INTFLAGS = 0xff;

	// get current value of display mode switch (V-I .vs. PWR-SoC)
	// (System will start in the V-I mode, then MODE-SW toggles screens)
	// GET PRESENT SW VALUE and STORE IT)
	dsp_mode_sw_val = PORTC.IN & PIN3_bm;  // Save current mode sw value
	dsp_mode_flag = 1;					   // (Either a '0' or an '8')
			// 0 = V-I display 
	portB_image_old = PORTB.IN;  // For RPG...
	mode_changed = 0;

		//Clear ALL Text Areas (leave lines!)
		oled1_setxt_position (4,6);   // Clr start position
		oled2_setxt_position (4,6);   // Clr start position
		oled3_setxt_position (4,6);   // Clr start position

		oled1_send_command (&oled_setxt_height_talltall[0]);  // set text height
		oled2_send_command (&oled_setxt_height_talltall[0]);  // set text height
		oled3_send_command (&oled_setxt_height_talltall[0]);  // set text height

		oled1_putstring (&ClrTextArea2[0]);    //
		oled2_putstring (&ClrTextArea2[0]);    //
		oled3_putstring (&ClrTextArea2[0]);    //



	// ==================================================================
	// Check TAILITE signal  (1=Night == RED-ISH TEXT)
	// Set  TEXT  COLOR      (0=Day == WHITE TEXT)
	// (Note tailite_flag == tailite "old state")
	//
	if (PORTC.IN & PIN5_bm) {  // Then tail lights ON
		//RED LETTERS & NUMBERS
//		OLD COLOR		
//		oled1_send_command (&oled_setxt_FG_colorRED[0]);  

		// #define night COLOR = OrangeRed = 0xFA20
		oled1_send_command (&oled_setxt_FG_colorORGRED[0]);
		oled2_send_command (&oled_setxt_FG_colorORGRED[0]);
		oled3_send_command (&oled_setxt_FG_colorORGRED[0]);
		tailite_flag = 1; // They are on
	}
	else {  // Tail lights off... WHITE
		oled1_send_command (&oled_setxt_FG_colorWHT[0]);  
		oled2_send_command (&oled_setxt_FG_colorWHT[0]);  
		oled3_send_command (&oled_setxt_FG_colorWHT[0]);  
		tailite_flag = 0; // They are off
	}

	oled1_send_command (&oled_drw_line_lo[0]);

	if (dsp_mode_flag == 0) {
		oled2_send_command (&oled_drw_line_lo[0]);
	}
	oled3_send_command (&oled_drw_line_lo[0]);

	// Set txt PARAMETERS title, etc....
	oled2_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_width[0]);
	ssc_oled_lines(); 

	battery_tasks();
	get_133V_battery_voltage();   
	
	if (dsp_mode_flag == 0)  {
		// determines/displays vehicle HV pack voltage and 
		// current, in real time.
		display_pack_voltage ();
		display_pack_current();
	}
	else  {
		soc_state_screen_load();
	}


	// START UP TASK AND PRESENT STATE INITIALIZATION
	// (CAN BE ALTERED TO ANY STATE FOR START UP)
	state_num = CONTROLLER_STATE;
	controller_tasks();

	load_tmparray_display ();
		//loads all current temps, updates current state temp

	contrast_level = 15;  	// BUT NOT SENT TO OLEDs YET...
					// BUT NOT SENT TO OLEDs YET...
					
	_delay_ms(10); //////////////////////////////////////////////////

	//Turn OFF servo power, PA6 = SVO-SSR = 0 IFF IGN = 0
	//	PORTA.OUTCLR = PIN6_bm;
    pedal_lock_pwr_off();

	tone();
	sei();
			
			
	/////////////////////////////////////////////////	
	/////////////////////////////////////////////////	
	//   Needed for first pass update ONLY
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	
	cli();
	verify_SOCH_online();
		
	//Wait for SOCH to be ready...
	_delay_ms(10);    // 20 worked...
	
	if (soch_offline_flag == 0)  {    // Then online, execute ...
        // Update SoCH values (Vpack, Ipack, SoC, kWh)
        SOC_UART2_SndCmd (&get_pack_voltage[0]);  //
		Get_SOC_Response(&pack_voltage_array[0], 'V');
		_delay_ms(5);

		SOC_UART2_SndCmd (&get_pack_current[0]);  //
		Get_SOC_Response(&pack_current_array[0], 'A');
		_delay_ms(5);

		SOC_UART2_SndCmd (&get_pack_soc[0]);  //
		Get_SOC_Response(&pack_soc_array[0], '%');
		_delay_ms(5);

		SOC_UART2_SndCmd (&get_watthours_soc[0]);  //
		Get_SOC_Response(&pack_kwh_array[0], 'W');
		_delay_ms(5);
	}
		
	sei();
					

  // L O W E R   I N F I N I T E   E V I M _ S T A T E = = = = = = = = = = = = 
  // L O W E R   I N F I N I T E   E V I M _ S T A T E = = = = = = = = = = = = 
  // L O W E R   I N F I N I T E   E V I M _ S T A T E = = = = = = = = = = = = 
  while (top_state_num == EVIM_STATE)  
	{

	if (charge_cycle_active_flag == 1) {
		mode_switch_counter++;  // increment mode switching counter
		if (mode_switch_counter == 14) {
			mode_changed = 1;
			dsp_mode_flag = dsp_mode_flag ^ PIN0_bm;
			mode_switch_counter = 0;
		}
	}

	cli();
	verify_SOCH_online();
		
	//Wait for SOCH to be ready...
	_delay_ms(10);    // 10 == Min Value

	if (charge_cycle_active_flag == 1) {
		if ((soch_offline_flag == 0) && (i_soch >= 4))  {  // Then online, Go... 
		  if (dsp_mode_flag == 0) { // V-I mode
			// Update SoCH values (Vpack, Ipack, SoC, kWh)
	        SOC_UART2_SndCmd (&get_pack_voltage[0]);  //
			Get_SOC_Response(&pack_voltage_array[0], 'V');
			_delay_ms(5);

			SOC_UART2_SndCmd (&get_pack_current[0]);  //
			Get_SOC_Response(&pack_current_array[0], 'A');
			_delay_ms(5);
			i_soch = 0; 
		  }
		else  {    // %SoC and kWh mode
			SOC_UART2_SndCmd (&get_pack_soc[0]);  //
			Get_SOC_Response(&pack_soc_array[0], '%');
			_delay_ms(5);

			SOC_UART2_SndCmd (&get_watthours_soc[0]);  //
			Get_SOC_Response(&pack_kwh_array[0], 'W');
			_delay_ms(5);
			i_soch = 0; 
		}
	  }
	}
	else if (soch_offline_flag == 0)  {  // Then online, Go... 
	  if (dsp_mode_flag == 0) { // V-I mode
        // Update SoCH values (Vpack, Ipack, SoC, kWh)
        SOC_UART2_SndCmd (&get_pack_voltage[0]);  //
		Get_SOC_Response(&pack_voltage_array[0], 'V');
		_delay_ms(5);

		SOC_UART2_SndCmd (&get_pack_current[0]);  //
		Get_SOC_Response(&pack_current_array[0], 'A');
		_delay_ms(5);
		i_soch = 0; 
	  }

	  else  {    // %SoC and kWh mode
		SOC_UART2_SndCmd (&get_pack_soc[0]);  //
		Get_SOC_Response(&pack_soc_array[0], '%');
		_delay_ms(5);

		SOC_UART2_SndCmd (&get_watthours_soc[0]);  //
		Get_SOC_Response(&pack_kwh_array[0], 'W');
		_delay_ms(5);
		i_soch = 0; 
	  }
	}
  

	// ************************************************************	
	// Charge Mode Slow Down Counter
	// The counter must reach compare value to run soch requests 
	// for data update
	// 	
	i_soch++; // increment counter for running soch request. 
	// ************************************************************	

	sei();
						
	/// SET OLED TEXT/NUMERIC CHARACTER COLOR
	// Get current state of Tailite Signal
	new_tail = PORTC.IN & PIN5_bm;
	if (new_tail == PIN5_bm) // If signal=1, new_tail=1
		new_tail = 1;
	else
		new_tail = 0;
	
		//////////    C O L O R  C H A N G E  C O D E    ///////////////////
		//////////    C O L O R  C H A N G E  C O D E    ///////////////////
		if (new_tail != tailite_flag) {                                   //
			cli();                                                        //
			// Set OLED display contrast = 1 (off, but still powered up). //
			contrast_oldval = contrast_level;                             // 
			contrast_level = 1;   // DEBUG VALUE == 3 Should be 1
			oled_contrast_set_cc(contrast_level);  // set to last setting

			if (tailite_flag == 1) {  // So, now tail lights *ARE* OFF
				oled1_send_command (&oled_setxt_FG_colorWHT[0]);   
				oled2_send_command (&oled_setxt_FG_colorWHT[0]);   
				oled3_send_command (&oled_setxt_FG_colorWHT[0]);   
				tailite_flag = 0;  // lights off
			}
			else { // tail lights are now on
				oled1_send_command (&oled_setxt_FG_colorORGRED[0]);   
				oled2_send_command (&oled_setxt_FG_colorORGRED[0]);
				oled3_send_command (&oled_setxt_FG_colorORGRED[0]);
				tailite_flag = 1; // lights on
			}
					
			// Reload based on current FG color & and state...
			switch (state_num) {
				case MOTOR_STATE :
					motor_tasks();
					break;
				case CONTROLLER_STATE :
					controller_tasks();
					break;
				case DCDC_STATE :
					dcdc_tasks();
					break;
				case BBOX1_STATE :
					bbox1_tasks();
					break;
				case BBOX2_STATE :
					bbox2_tasks();
					break;
				default :  // Ambient, but not possible?
					break;
			}
					
			battery_tasks();
			get_133V_battery_voltage();   
				// determines/displays vehicle accessory battey
				// voltage, to the 100th fo a volt, in real time.
	
			load_tmparray_display ();
			if (dsp_mode_flag == 0) { // V-I mode
							
				// Re-draw center line...
				oled2_send_command (&oled_drw_line_lo[0]);
	
				display_pack_voltage ();
				display_pack_current();
			}
			else  {
				// Or... loads soc and kwh display
				soc_state_screen_load();
				oled2_setxt_position (10,70);
			
				display_pack_soc();
				display_pack_kwh();
				get_a12V_voltage();
				get_a5V_voltage();			
			}
			
		  	contrast_level = contrast_oldval;
			oled_contrast_set_cc(contrast_level);     //// DIAG DIAG
			sei();	

		} // End of text color change IF statement                  ///
			  // -------------------------------------                  ///
			  // -------------------------------------                  ///
			  /////////////////////////////////////////////////////////////
			
	
		// OLED1 UPDATE... Temperature Display
		cli ();
		load_tmparray_display ();   
				//loads all current temps, updates current state temp 

		sei();
		// ENABLE Pending Intr's to execute
   		cli();

		// Check for CONSTRAST_LEVEL change,
		if (contrast_level != contrast_oldval) {
			contrast_oldval = contrast_level;				
			oled_contrast_set_cc(contrast_level);
		}				// set to last/new setting

		if (mode_changed == 1) {  // must change OLED2 content...
			//Turn DOWN OLED display contrast (1 == OFF while still powered up).
			contrast_oldval = contrast_level;
			contrast_level = 1;   // DEBUG VALUE == 3 Should be 1
			
			oled2_send_command (&oled_setxt_width[0]);
			clr_text_area2_short();   
						// clears the large text area of OLED2

			if (dsp_mode_flag == 0) { // V-I mode
				// Re-draw center line...
				oled2_send_command (&oled_drw_line_lo[0]);
				display_pack_voltage ();
				display_pack_current();
			}
			else  {
				// Or... loads soc and kwh display
				soc_state_screen_load();
				oled2_setxt_position (10,70);
				display_pack_soc();
				display_pack_kwh();
				get_a12V_voltage();
				get_a5V_voltage();			
			}
			mode_changed = 0;  // reset for next interrupt...

			//Turn DOWN OLED display contrast (1 == OFF while still powered up).
			contrast_level = contrast_oldval;
			
			oled2_contrast_set(contrast_level);  // set to last setting, as
		}
			
		else {  // NO MODE CHANGE
			// determines/displays vehicle HV pack voltage and
			// current, in real time.
			if (dsp_mode_flag == 0)  {
				//loads all current temps, updates current state temp
				display_pack_voltage ();
				display_pack_current();
				}
			else  {
				// Or... loads soc and kwh display
				oled2_setxt_position (10,70);  

				display_pack_soc();
				display_pack_kwh();
				get_a12V_voltage();
				get_a5V_voltage();			
				}
			}

		// Check for CONSTRAST_LEVEL change,
   		if (contrast_level != contrast_oldval) {
      		contrast_oldval = contrast_level;
	  		oled_contrast_set_cc(contrast_level);  
			}				// set to last setting
   		sei();

		// OLED3 UPDATE... ACCESSORY BATTERY VOLTAGE 
		cli ();
		get_133V_battery_voltage();   
			// determines/displays vehicle accessory battey
			// voltage, to the 100th fo a volt, in real time.

		sei();
		// ENABLE Pending Intr's to execute
		cli ();

		//	END CHECKS... cHRG/EVIM mode active still active?	
		if ((charge_cycle_active_flag == 0) && (evim_state_active_flag == 0))  {
            top_state_num = STANDBY_STATE;   //09232021
			contrast_level = 1;   // Max value
			oled_contrast_set_cc(contrast_level);
		}	// Heading back to standby_state...
		sei();
					
	} // end of LOWER EVIM_STATE WHILE LOOP
 } // end of VIM_STATE UPPER while loop


}  // END of While(1)  

}  // End of MAIN    


























//****************************************************************
// get_133V_battery_voltage (void)                    AVR128DA48
//
// Description:	This routine converts the voltage on ADC7 (PD7),
//				which is the accy 13.3V lithium battery voltage, 
//				and displays the result on OLED1 (left display)
//              It also loads/updates the associated global
//              variables.  
//****************************************************************
void get_133V_battery_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
    uint16_t tens;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN7_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD7 = ADC7
	ADC0_MUXPOS = 0x07;  // Selects channel6, which is accy batt voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	  // Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	  // ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
							 
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low) +30);

	multiplier = (uint16_t) 1000;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	current_batt_voltage = (product / ((uint16_t) 272));

	// -------------------------------------------
	// OFFSET/Correction Adjustment...  
	// current_batt_voltage = current_batt_voltage+150;  // ~ 0.15 volt increase!
	// 12/16/22 current_batt_voltage = current_batt_voltage - 80;
	current_batt_voltage = current_batt_voltage - 110;  // Due to in car shift?!	
	// -------------------------------------------
	
	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1

	// OPTIONAL NOISE REDUCING CODE...
    //// current_batt_voltage = current_batt_voltage >> 3; // for ACCUM feature

	
	// Convert to BCD digits for display 
	// (process & separate digits)
	current_batt_voltage = current_batt_voltage/10; // reduce mv to TENTHS of mV
	hundredths = current_batt_voltage%10;
	accy133_hundredths = hundredths;
	   
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;
	accy133_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	accy133_units = units;
	
	current_batt_voltage = current_batt_voltage/10;
	tens = current_batt_voltage;
	accy133_tens = tens;
			    
	//Add the " V" for voltage...
    oled1_setxt_position (110,78);  // was 110
    putcharOLED1 ('V');
    oled1_setxt_position (33,78);
 		   
    if (tens != 0)
       putcharOLED1 (0x30 + tens);
	else
	   putcharOLED1 (' ');
		
	   putcharOLED1 (0x30 + units);
	   putcharOLED1 ('.');
	   putcharOLED1 (0x30 + tenths);
	   putcharOLED1 (0x30 + hundredths);
}
		




//****************************************************************
// get_a12V_voltage (void)                    AVR128DA48
//
// Description:	This routine converts the HV auxillary 12V supply on
//				ADC6 (PD6), which is the aux HV-to-12V SOCH and SDT
//				voltage, and displays the result on Soc_kWh screen
//****************************************************************
void get_a12V_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
    uint16_t tens;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN6_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD6 = ADC6
	ADC0_MUXPOS = 0x06;  // Selects channe6, which is aux 12V voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	// Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	// ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low) +30);

	multiplier = (uint16_t) 1000;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	current_batt_voltage = (product / ((uint16_t) 272));

	//////// OFFSET/Correction Adjustment...  
	//////// OFFSET/Correction Adjustment...  
	current_batt_voltage = current_batt_voltage + 200; // ~ 0.200 volts added
	//////// OFFSET/Correction Adjustment...  
	//////// OFFSET/Correction Adjustment...

	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1

	// OPTIONAL NOISE REDUCING CODE...
    //// current_batt_voltage = current_batt_voltage >> 3;  // for ACCUM reduction
	
	//Convert to BCD digits for display 
	current_batt_voltage = current_batt_voltage/10; // reduce mvs to 1/10s of mV
	hundredths = current_batt_voltage%10;   // process & separate digits
	aux12_hundredths = hundredths;
	
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;
	aux12_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	aux12_units = units;
	
	current_batt_voltage = current_batt_voltage/10;
	tens = current_batt_voltage;
	aux12_tens = tens;

	// Set txt PARAMETERS for voltage display
	oled2_send_command (&oled_setxt_height_sml[0]);
	oled2_send_command (&oled_setxt_width_narrow[0]);

	//Add the " V" for voltage...
	oled2_setxt_position (126,102);
    putcharOLED2 ('V');
	
	// Set position for value output
    oled2_setxt_position (95,102);    // was 100,104
 		
	// tens digit, and leading 0!	  	   
    if (tens == 0) {
		putcharOLED2 (' ');
		}	
	else {
		putcharOLED2 (0x30 + tens);
		}

	putcharOLED2 (0x30 + units);
	putcharOLED2 ('.');
	putcharOLED2 (0x30 + tenths);
	}





















//********************************************************************
// get_a5V_voltage (void)                            AVR128DA48
//
// Description:	This routine converts the voltage on ADC18 (ADC0.18),
//              Pin PF2, which is the HV based 5V auxillary voltage,
//				and displays the result 
//********************************************************************
void get_a5V_voltage (void)
  {
	uint16_t adc_result_int, multiplier, current_batt_voltage;
	uint32_t product;   // 4 byte product
	uint8_t adc_result_low; //unsigned byte variables.
            
	uint16_t hundredths; 
	uint16_t tenths;
	uint16_t units;
 
	// Set ADC Enable bit to 1...
	ADC0_CTRLA = 0; //reset all bits...
	ADC0_CTRLA |= (PIN6_bm | PIN0_bm);  // Run always = 1 & 
										// Enable = 1
	// Select PD6 = ADC6
	ADC0_MUXPOS = 0x12;  // Selects channe6, which is aux 12V voltage
	 	 
	// Set Vref == 4.096 volts, and is always on...
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL1_bm ; 

	// OPTIONAL NOISE REDUCING CODE...
	// Set ACCUM == 4 (four samples returned)	// OPTIONAL NOISE REDUCING CODE...
	// ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;		// OPTIONAL NOISE REDUCING CODE...
	
	// Small delay .... ????
	_delay_ms (1);
	 
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	                           // Bit-0 is CLRd when converion complete
	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	 
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low));  
													// correction if needed +30);
	multiplier = (uint16_t) 1467;
	product = ((uint32_t)adc_result_int * (uint32_t)multiplier);
	//current_batt_voltage = (product / ((uint16_t) 272));
	current_batt_voltage =   (uint16_t) (product / ((uint32_t) 1000));

	// -----------------------------------------------------------------
	//OFFSET/Correction Adjustment...  
	//    current_batt_voltage = current_batt_voltage + 50; // ~ 0.05 volts added!
	current_batt_voltage = current_batt_voltage; // ~ 0.00 volts added!
	// -----------------------------------------------------------------
	
	// Disable ADC... Enable = 0... Res = reset state
	ADC0_CTRLA = 0;  // Enable is bit 1 == 1

	//Convert to BCD digits for display 
	current_batt_voltage = current_batt_voltage/10; // reduce mvs to tenths of mV
	hundredths = current_batt_voltage%10;   // process & separate digits
	aux5_hundredths = hundredths;
	
	current_batt_voltage = current_batt_voltage/10;
	tenths = current_batt_voltage%10;  
	aux5_tenths = tenths;
	
	current_batt_voltage = current_batt_voltage/10;
	units = current_batt_voltage%10;
	aux5_units = units;

	// Set txt PARAMETERS for voltage display
	oled2_send_command (&oled_setxt_height_sml[0]);
	oled2_send_command (&oled_setxt_width_narrow[0]);
    oled2_setxt_position (25,102);   // was 30,104

	putcharOLED2 (0x30 + units);
	putcharOLED2 ('.');
	putcharOLED2 (0x30 + tenths);
	putcharOLED2 (0x30 + hundredths);

	//Add the " V" for voltage...
	oled2_setxt_position (57,102);
    putcharOLED2 ('V');
	}
		
		
//===============================================================================
//		Temperatue converting (analog) channels:
//			ADC0.0   PD0 = Motor NCT10K sensor (Warp11 Netgen DC Series Motor)
//			ADC0.1   PD1 = DC HV Controller NCT10K sensor (Raptor 1200 controller)
//			ADC0.2   PD2 = DC-DC Converter Heat Sink Plate NTC sensor
//			ADC0.3   PD3 = Battery Box #1 NCT10K sensor (midcar)
//			ADC0.4   PD4 = Battery Box #2 NCT10K sensor (trunk)
//			ADC0.5   PD5 = Ambient NCT10K sensor (in front of battery)
//
//		Voltage Reference: 
//			  All temperature conversions use the 2.56V reference!
//			  (Internal 2.56V reference)
//		ADC Resolution:
//			  Convesions are all 10-bits
//==============================================================================


// ****************************************************************
// get_temps (void)     LOADS TEMPS_ARRAY WITH CURRENT TEMPS
//
// int16_t returned is error indicator (TO BE ADDED)
// ****************************************************************
void get_temps (void)
{
  int16_t adc_val;
  int16_t channel_num=0;

  while (channel_num < 6)
    {
      adc_val = get_adc_ntc10k(channel_num);  // TWICE !!!!!
      adc_val = get_adc_ntc10k(channel_num);  // Seems to be req'd
                                         // INVESTIGATE THIS
										 // TRY ADDING DELAY @ TOP of LOOP...............................
      current_temps_array[channel_num]= adc_val;
      channel_num++;
    }
}


// ****************************************************************
//  int16_t get_adc_ntc10k (uint16_t channel_number)
//
//  Temp = (ADC# * SLOPE)/100 - INTERCEPT
// ****************************************************************
 int16_t get_adc_ntc10k (uint16_t channel_number)
  {
    uint16_t adc_result_int;
    uint8_t adc_result_low; //unsigned byte variables.

	// Set ADC Config (On, 12Bit, Enabled
	// Bit7 = NoSleep;  Bit2 = 10bit Res;  Bit0 = ADC Enable;
	ADC0_CTRLA =0; // clear previous bits... 	
	ADC0_CTRLA |= (PIN7_bm | PIN0_bm); 	
	
	ADC0_MUXPOS = (char) (0x00 | channel_number);
	
	// Set Vref == 2.5 volts, and is always on...
	// (VREF_REFSEL_2V500_gc = (0x03<<0))
	VREF_ADC0REF = VREF_ALWAYSON_bm | VREF_REFSEL_2V500_gc;  // 2.5V Selection

	// OPTIONAL NOISE REDUCING CODE...
	//// Set ACCUM == 4 (four samples returned)
	//// ADC0_CTRLB |= ADC_SAMPNUM_ACC8_gc;
	
	// Small delay .... ????
	_delay_ms (1);
	
	// START CONVERSION...
	ADC0_COMMAND |= PIN0_bm;  // Start a conversion
	// Bit-0 is CLRd when converion complete

	// Wait for END-OF-CONVERSION (Bit-0 cleared)
	while (ADC0_COMMAND & PIN0_bm) {};  // Bit-0 == 0 == EOC
	
	//Conversion complete... Build result int, and return
	adc_result_low = ADC0.RESL;   //get low byte, load high byte into TEMP
	adc_result_int = (uint16_t) ((((ADC0.RESH) << 8) | adc_result_low) +30);
    return (adc_result_int);     
   }




// ************************************************************************
// AVR128DA48 12-Bit ADC Temperature Interpolation Table
// Based on 4.7K fixed resistor as Top V-Divider Resistor
// ------------------------------------------------------------------------
//					ADC#	SLOPE	INTERCEPT   TEMP
const  int16_t interp_tbl[42]  =	 
				{	3836,	-92,	3600,   // 14F (-10C)	// was 91 3632
					3638,   -67,	2700,	// 32F (0C)		// was 66 2732
					3366,	-53,	2239,	// 50F (10C)	// was 2269
					3024,	-46,	2056,	// 68F (20C)	// was 2056
					2628,	-43,	1993,	// 86F (30C)
					2211,	-44,	2022,	// 104F (40C)
					1806,	-49,	2110,	// 122F (50C)   // WAS 2110
					1440,	-58,	2235,	// 140F (60C)   // was 2235
					1130,	-71,	2484,	// 158F (70C)   // WAS 2384
					877,	-91,	2600,	// 176F (80C)	// wasd 2554
					678,	-118,	2789,	// 194F (90C)	// was 2739
					525,	-144,	2924,   // 212F (100C)	// was 2876
					463,	-144,	2940,	// 221F (105C)  // was 2890
					 0,		  0,     0   }; // END TABLE CATCH !


// Single TEMP value scaling...
//
// Temp = ((ADC# * SLOPE)/100) - INTERCEPT
 int16_t scale_temp_ntc10k ( int16_t adc_result_12bit)
  {
	int16_t  y_intercept, slope, adc_boundary_num;   //  *region_tbl_ptr, 
	int16_t tmp_in_tenths;
	int32_t slope_32bit, adc_32bit_val;
	int32_t product;
	const  int16_t *region_ptr;
    
    //load adc result for scan of segment/region
    adc_32bit_val = (int32_t) adc_result_12bit;

    //init pointer to scan for segment/region assoc values
    region_ptr = &interp_tbl[0];

    //Scan to find region based on passed raw value...
    adc_boundary_num = *region_ptr++; // load first boundary

    if (adc_32bit_val > 4000)  {   //3638
		 tmp_in_tenths = 0x0fff;    // return and display *LOW*
	 	 //return;
		 return (tmp_in_tenths);
		 }
	else if (adc_32bit_val < 463) { // return and display *HIGH*
         tmp_in_tenths = 0;
		 //return;
		 return (tmp_in_tenths);
		 }
	else {
		while (adc_32bit_val < (int32_t) adc_boundary_num) {
			*region_ptr++;  // Adjust to next boundary #
			*region_ptr++;
			adc_boundary_num = *region_ptr++; // load first boundary
			}     
		}
	
    //OK found line... Lookup values in linearization table
    slope = *region_ptr++;   //load slope from current line
    y_intercept = *region_ptr++;  // load Y (temp) axis intercept
    slope_32bit = (int32_t) slope;
    product = adc_32bit_val * slope_32bit;
    tmp_in_tenths = (int16_t)((int16_t)(product/100) + y_intercept);
    return (tmp_in_tenths);
}



// Full temp array scaled and stored in global array for Wi-Fi 
// remote access
//

// ****************************************************************
// scale_temps_array (void)     Scales and loads SCALED_TEMPS_ARRAY 
//                              with CURRENT temps in array
//
// ****************************************************************
void scale_temps_array (void)
{
	int16_t temp_val;
	int16_t channel_num=0;

	while (channel_num < 6)
	{
		temp_val = current_temps_array[channel_num];
		scaled_temps_array [channel_num] = scale_temp_ntc10k (temp_val);
		channel_num++;
	}
}
 










	
//=============================================================================
//       ======================================================
//=======  V A R I O U S     E V I M     S U B R O U T I N E S ================
//       ======================================================
//=============================================================================	

// load_evim_screen_lines (void)
// Description: Loads temp display frame and line
// Inputs: None
// Outputs: LCD OLED Boxes and center line (in RED)
// ------------------------------------------------------------
void load_evim_screen_lines (void)
	{
	cli();

	//clear the displays
	oled1_send_command (&oled_clr_scrn[0]);
	oled2_send_command (&oled_clr_scrn[0]);
	oled3_send_command (&oled_clr_scrn[0]);
   		
	// Draw inner & outer edge rectangle, & horizontal partition line...
	oled1_send_command (&oled_drw_rect_outer[0]);	// OLED1 lEFT
	oled1_send_command (&oled_drw_rect_inner[0]);	
	oled1_send_command (&oled_drw_line_lo[0]);	

	oled2_send_command (&oled_drw_rect_outer[0]);	// OLED2 MIDDLE
	oled2_send_command (&oled_drw_rect_inner[0]);
	
	if (dsp_mode_flag == 0) {
		oled2_send_command (&oled_drw_line_lo[0]);
	}
		
	oled3_send_command (&oled_drw_rect_outer[0]);	// OLED3 RIGHT
	oled3_send_command (&oled_drw_rect_inner[0]);
	oled3_send_command (&oled_drw_line_lo[0]);

	// Set txt PARAMETERS title, etc....
	oled2_send_command (&oled_setxt_height[0]);
	oled2_send_command (&oled_setxt_width[0]);
	sei();
}
	



// --------------------------------------------------------
// load_tmparray_display (void)
// Description: Loads the entire temp array with current
//      temperatures, and displays the current temp)
//
// Inputs: None
// Outputs: tmparray is updated
//          current state temp is displayed
// --------------------------------------------------------
void load_tmparray_display (void)
{
	uint16_t adc_current_val;
	 int16_t current_scaled_temp;  // CNT NEEDED???
	 int16_t *ptr;  // To TempsArray
			   
		// get temps... load array   
        // ****************************************************
	       get_temps();    // Load all current temps into array
		   scale_temps_array();
   
       //    display current state temp   
   	   //******************************************************
		ptr = &current_temps_array [state_num];
	    adc_current_val = *ptr;   // get current temp for state_num 
	    current_scaled_temp = scale_temp_ntc10k (adc_current_val);

	    //display current scaled temperature
		oled3_send_command (&oled_setxt_height[0]);
		oled3_send_command (&oled_setxt_width[0]);
	    oled3_setxt_position (45,78);
	    dsp_ps_temp (current_scaled_temp);  //
		   
} // RETURN



// TEMP DISPLAY RELATED
// *************************************************************************
// dsp_ps_temp
// Desc: Displays present state temperature on bottom half of the OLED LCD
// 
// Inputs:
// Outputs:
//
// *************************************************************************
void dsp_ps_temp ( int16_t temp2disp)
{
     //Variable definitions
	 
      int16_t tenths;
      int16_t units;
      int16_t tens;
      int16_t hundreds;
      int16_t temp_celcius;

	///// cli();
      int16_t tmp = temp2disp;
          
     static  uint16_t current_digit_cnt=0;
     static uint8_t current_sign = 0;  // assume sign is pos = 0, neg=1; 
     static uint8_t need_neg_sign = 0;  // assume positive    
     uint8_t units_flag=1;   // 1=F, 0=C

	 // CHECK FOR OUT OF RANGE INDICATIONS: 0 (LOW) OR 0xfff (HIGH)
     if (tmp == 0) { // HIGH temp OOR
	       temp_high_tasks();
		   return;
	 }
	 else if (tmp == 0xfff) {
	       temp_low_tasks();
		   return;	  
			}
	 else 
	  {
			if ((PORTC.IN & PIN2_bm) == 0)  // PC0=0 ... Then convert to celsius
				{
					temp_celcius = ((tmp-320)*5)/9;
					tmp = temp_celcius;
					units_flag = 0;  //use a C and not an F
				}
			//check sign of new value to display
			if (tmp < 0) { //negative
				current_sign = 1; 
				}
			else current_sign = 0;  // positive

		     if (current_sign != sign) {
			    sign = current_sign;
				clr_tmp_dsp_area();
				}        

	     //Pre-process tmp value: Check for negative, sign disp, and zero value
		if (tmp < 0) {
			need_neg_sign = 1;    // neg sign now needed
			tmp = -tmp;  // now the magnatude will be displayed
			current_digit_cnt++;  //first char needed...
			}
		else if (tmp == 0){   // turn off '-' is present
			need_neg_sign = 1;    // neg sign now needed
			}
		else { //then it is positive
			   need_neg_sign = 0;    // neg sign now needed
			}
          
		//determine if digit count changed, and temp dsp area
		//needs to be cleared...
		if (tmp > 999)           //FOUR DIGITS to display
			current_digit_cnt =+ 4;
		else if (tmp > 99)       // THREE DIGITS to display       
			current_digit_cnt =+ 3;
	    else                     // ONE DIGIT to display
		    current_digit_cnt =+ 2;
    
	    if (dig_cnt != current_digit_cnt) {
		    clr_tmp_dsp_area();
			dig_cnt = current_digit_cnt;
			}

	    if (need_neg_sign == 1) {
			oled3_send_command (&oled_setxt_height[0]);
			oled3_send_command (&oled_setxt_width[0]);
		    oled3_setxt_position (15,78);
			putcharOLED3 ('-');
			}
      
	    //FOUR DIGITS to display    
		if (tmp > 999) {
			tenths = tmp%10;   // process & separate digits
			tmp = tmp/10;
			units = tmp%10;
			tmp = tmp/10;
			tens = tmp%10;
		    tmp = tmp/10;
			hundreds= tmp;

	        //------------------
		    
			oled3_send_command (&oled_setxt_height[0]);
 			oled3_send_command (&oled_setxt_width[0]);
			oled3_setxt_position (32,78);      
			putcharOLED3 (0x30 + hundreds); 
	        putcharOLED3 (0x30 + tens);
		    putcharOLED3 (0x30 + units);
			putcharOLED3 ('.');
	        putcharOLED3 (0x30 + tenths);
        
		    //Do "Degrees"  & "F" notation....
			oled3_setxt_position (106,70);   //was 103,70
	        oled3_send_command (&oled_setxt_height_sml[0]);
		    putcharOLED3 ('o');
			oled3_send_command (&oled_setxt_height[0]);
	        oled3_setxt_position (120,78);
		   }
    
		// THREE DIGITS to display       
		else if (tmp > 99)  {   
			tenths = tmp%10;
	        tmp = tmp/10;
		    units = tmp%10;
			tmp = tmp/10;
	        tens = tmp;

		    //------------------
	        oled3_setxt_position (36,78);
		    putcharOLED3 (0x30 + tens);
			putcharOLED3 (0x30 + units);
	        putcharOLED3 ('.');
		    putcharOLED3 (0x30 + tenths);
        	
		    //Do "Degrees"  & "F" notation....
			oled3_send_command (&oled_setxt_width[0]);
			oled3_setxt_position (106,70); 
			oled3_send_command (&oled_setxt_height_sml[0]);
		    putcharOLED3 ('o');
	        oled3_send_command (&oled_setxt_height[0]);
	        oled3_setxt_position (120,78); 
		    }

	    else {    // TWO DIGITs to display
		    tenths = tmp%10;
			tmp = tmp/10;
	        units = tmp;
	
		    //------------------
			oled3_send_command (&oled_setxt_height[0]);
			oled3_send_command (&oled_setxt_width[0]);
	        oled3_setxt_position (32,78);
		    putcharOLED3 (' ');  // For cleanup...
			putcharOLED3 (0x30 + units);
	        putcharOLED3 ('.');
		    putcharOLED3 (0x30 + tenths);
			
            // ADDED SPACE 06102020
	        putcharOLED3 (' ');

			//Do "Degrees F" notation....
	        oled3_setxt_position (98,70);   //was  98,70
		    oled3_send_command (&oled_setxt_height_sml[0]);
			putcharOLED3 ('o');
	        oled3_send_command (&oled_setxt_height[0]);
		    oled3_setxt_position (112,78);
			}
    
	        if (units_flag == 1)
				putcharOLED3 ('F');
			else 
				putcharOLED3('C');
		}
		/////sei();
}  // END OF SUBR




// *****************************************************************
// (void) Clr_Tmp_Dsp_Area (void)
// *****************************************************************
void clr_tmp_dsp_area (void)
   {
      oled3_setxt_position (15,70);   //was 78
	  oled3_send_command (&oled_setxt_height_tall[0]);
	  oled3_putstring (&blnk_strng[0]);  // ~ 30mS execution time...
      _delay_ms(30);
      oled3_send_command (&oled_setxt_height[0]);
   }

