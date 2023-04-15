/********************************************************************************/
// EVIM_AVR128 INTERRUPT HANDLER ROUTINGS - PLUS TITLES FOR OLED TOP AREAS
// *** RELEASE 2 ***
//
// Reduced/Minimized Switch-Case Version
// (CONCURRENT ROUTINES ADDED 02-2021)
//
// This INCLUDE file contains all software required for the interrupt handling
// and the actual control/task subroutines.  The FSM flow control is "switch"
// statement based, and handles both the Top Level FSM, and the EVIM (Electric 
// Vehicle Instrument Monitor) FSM.
//
// PORTA interrupt vector == Charge Active Signal == PA4
//
// PORTB interrupt vector == RPG Signals = CH-A (PB2), CH-B (PB3), PBSW (PB4)
//
// PORTC interrupt vector == IGN (PC7)
//
// PORTE senses the contactor "Cntctr" (PE0)
//
//
// TCA00 ISR is for a 15 minute timeout software-reset feature 
/***************************************************************************/



// NE W    2 0 2 3    V E R S I O N 
//
//***************************************************************************
// Function Name        : "TCA0 ISR"
// Description : Each interrupt increments a timeout count. Once the timeout
//      count reaches "138", the system returns to STANBY_STATE by executing
//      a software reset.
//
// //  timeout_counter val defs:  7 ~= 1 min; 37 ~= 5 mins;
//**************************************************************************
ISR(TCA0_OVF_vect) {
	cli();
	timeout_counter += 0x01; // increment timeout counter (~6.5 sec / Intr)
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; //clear interrupt flags
	
	if (rpg_on_flag == 1) { // Shorter count for return to STBY state
		if(timeout_counter >= 7) {   // 
			// Perform SOFTWARE Reset operation
			_PROTECTED_WRITE(RSTCTRL.SWRR, PIN0_bm);
		}
	}
	else {  // Then door open
		if(timeout_counter >= 37) {   // 
			// Perform SOFTWARE Reset operation
			_PROTECTED_WRITE(RSTCTRL.SWRR, PIN0_bm);
		}
	}
	sei();  //12/15/2022
}






//***************************************************************
// CHARGE INPUT INTERRUPT HANDLER
// (Using PORTA, Bit PA4 = CRG Signal)
//***************************************************************
ISR (PORTA_PORT_vect)
  {

  cli(); 

	// debounce delay
	_delay_ms(50);    // Bounce delay
	PORTA_image = PORTA.IN;  // get trigger bit values
	
	// Could be START or END of charge cycle...
	if ((PORTA_image & PIN4_bm) != 0) {  // charge cycle STARTING?
		charge_cycle_active_flag = 1; // start of charge cycle
		
		}
	else { // MUST BE END OF CHRG CYCLE
		charge_cycle_active_flag = 0; 
		}
		
		
    // clear all pending intr...
	PORTA.INTFLAGS = 0xff; 
	PORTB.INTFLAGS = 0xff;
	PORTC.INTFLAGS = 0xff;
	PORTE.INTFLAGS = 0xff;
	sei();
	return;
}  // END ISR





   
// ISR (PORTB)
// ***  MAIN ROTORY PULSE GENERATOR SOFTWARE DRIVER/INTERFACE  ***  
// *************************************************************** 
// Inputs from RPG:
//   RPG-CHA == PB2
//   RPG-CHB == PB3
//   RPG-Pushbutton == PB4
//      Active High [pressed=closed=VCC], Logic-1
//	    Normally Open, w/external 10K pulldown == Logic-0
//
//    Rotating the RPG knob switches temp monitoring sensors
//    (Knob may be rotated CW (advance) or CCW (backup))
//
//	  Pressing Knob == Activating RPG PB-Switch == Logic-1 (pressed)  
//       (Rotating "pressed" knob alters OLED display intensity)
//

ISR(PORTB_PORT_vect)    // PORTB RPG Trigger
  { 
	cli();

 	// DEBOUNCE DELAY - Adjusted 07/07/20
	_delay_ms(30);    // WAS 50
	 
    //ENTRY location... get current port image
    portB_image = PORTB.IN;   //PD7=CHB, PD6=CHA, PD5=PBSW

	// If RGP_On_Mode, reset "timeout_counter" due to activity...
	if ((rpg_on_flag == 1) && (charge_cycle_active_flag == 0)) {
		TCA0_stop();  // Stop timeout routine
		TCA0_init();  // Restart timeout routine
	}

	
	// if    IGN=0     &    RPG-PB pressed   & rpg_on_flag = 0
	//   AND we are not in charge mode...
	if ((!(PORTC.IN & PIN7_bm)) && (!(PORTB.IN & PIN4_bm)) 
	     && (rpg_on_flag == 0) && (charge_cycle_active_flag == 0)) {
		rpg_on_flag = 1;
		warm_restart_flag = 1;
		tone();  //  12/16...
	}
	
    //Check the RPG PBSW signal (PB4) Normally = 0; logic-1 if pressed
	//    if ((portB_image & PIN4_bm) && (ambient_active != 1))  { 
	else if ((portB_image & PIN4_bm) && (ambient_active != 1))  {
		tone();   //
        interrupted_state = state_num; // save for return
        state_num = AMBIENT_STATE;  // make AMBIENT active...
        ambient_active = 1;    //set flag active...
        ambient_tasks();  // Load top of display, plus

	    // clear all pending intr...
		PORTA.INTFLAGS = 0xff;  
		PORTB.INTFLAGS = 0xff;
		PORTC.INTFLAGS = 0xff;
		PORTE.INTFLAGS = 0xff;		   
        return;
    }					//PBSW=1=pressed, and Ambient Active
    else if ((ambient_active == 1) && (portB_image & PIN4_bm)) {

        // CHECK FOR RPG ACTIVITY (During AMBIENT State)!
        temp_port_image = portB_image & (PIN2_bm | PIN3_bm); // mask all but ChB & ChA
        if (temp_port_image != portB_image_old) {
            direction_flag = 1;   //CW
            switch (temp_port_image) {
               case 0x00 :
                  if ((portB_image_old & 0x0c) == 0x08) 
                     direction_flag = 0;   //CCW
                     break;
               case 0x04 :
                  if ((portB_image_old & 0x0c)  == 0x00) 
                     direction_flag = 0;   //CCW
                     break;                 
               case 0x0c :
                  if ((portB_image_old & 0x0c)  == 0x04) 
                     direction_flag = 0;   //CCW
                     break;
               case 0x08 :
                  if ((portB_image_old & 0x0c)  == 0x0c) 
                     direction_flag = 0;   //CCW
                     break;
		       default : {}; 
            }
			 
        portB_image_old = portB_image;   // update current port bits
                                         // for next interrupt             
        }  

	tone();   //   12/16 added

	    //   ***** With Direction determined... Process rotation...
            if (direction_flag) { //CCW = Reduce Contrast
              if (contrast_level >= 5) {
                 contrast_level--;  // decrease current level
                 contrast_level--;  // decrease current level
                }
              }
            else {  //CW = Increase contrast
              if (contrast_level <= 13) {
                 contrast_level++;  // increase current level
                 contrast_level++;  // increase current level
                 }				 
            }
	// clear all pending intr...
	PORTA.INTFLAGS = 0xff;
	PORTB.INTFLAGS = 0xff;
	PORTC.INTFLAGS = 0xff;
	PORTE.INTFLAGS = 0xff;
    return;
    }

	// RPG-PB was Released...	Clean up and return
    else if ((ambient_active == 1) && !(PORTB.IN & PIN4_bm)) {                                               
	      tone();   //   12/16 added
		  ambient_active = 0;
          state_num = interrupted_state; // return to interrupted stat

          // Restore display based on current present_state...
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
		 //PCIFR = 0x0f;   // clear all pending intr...
		 PORTA.INTFLAGS = 0xff;    // clear them all!
		 PORTB.INTFLAGS = 0xff;
		 PORTC.INTFLAGS = 0xff;
		 PORTE.INTFLAGS = 0xff;
         return;
     }

	// ** OK... Must be edge on ChA or ChB, PROCESS & determine rotation
        // ** direction by comparing Old_Val with the New_Val...
	 channel_d3_d2 = portB_image & 0x0c; // mask all but ChB & ChA

     direction_flag = 1;   
        switch (channel_d3_d2) {
           case 0x00 :
              if ((portB_image_old & 0x0c) == 0x08) 
                  direction_flag = 0;   //CCW
                  break;
           case 0x04 :
              if ((portB_image_old & 0x0c)  == 0x00) 
                  direction_flag = 0;   //CCW
                  break;                 
           case 0x0c :
              if ((portB_image_old & 0x0c)  == 0x04) 
                  direction_flag = 0;   //CCW
                  break;
           case 0x08 :
             if ((portB_image_old & 0x0c)  == 0x0c) 
                  direction_flag = 0;   //CCW
                  break;
           }
          
     portB_image_old = portB_image;  // update for next interrupt             
                  
     
	 //With Direction determined... Process click!
	 if ((ambient_active == 0) && (state_num != 0)) {
	    tone();   // 12/16
		switch (state_num) {
		   case MOTOR_STATE :
			   if (direction_flag) { //CCW
				   state_num = BBOX2_STATE;
				   bbox2_tasks();
			   }
			   else {   //CW
				   state_num = CONTROLLER_STATE;
				   controller_tasks();
			   }
			   break;
		   case CONTROLLER_STATE :
               if (direction_flag) { //CCW
                   state_num = MOTOR_STATE;
                   motor_tasks();
               }
               else {   //CW
                   state_num = DCDC_STATE;
                   dcdc_tasks();
               }
               break;
		   case DCDC_STATE :
               if (direction_flag) { //CCW
                   state_num = CONTROLLER_STATE;
                   controller_tasks();
               }
               else {   //CW
                   state_num = BBOX1_STATE;
                   bbox1_tasks();
               }
               break;
		   case BBOX1_STATE :
               if (direction_flag) { //CCW
                   state_num = DCDC_STATE;
                   dcdc_tasks();
               }
               else {   //CW
                   state_num = BBOX2_STATE;
                   bbox2_tasks();
               }
               break;
		   case BBOX2_STATE :
               if (direction_flag) { //CCW
                  state_num = BBOX1_STATE;
                  bbox1_tasks();
               }
               else {   //CW
                  state_num = MOTOR_STATE;
                  motor_tasks();
              }
              break;
          
           //error trap 
           default:     
              break;
        } // end of Switch-Case
  } // end of "click processing"

	 
  // ***********  IMPORTANT   IMPORTANT  ********************
  // *********** Do Not Remove This Code ********************
		  PORTA.INTFLAGS = 0xff;    // clear them all!
		  PORTB.INTFLAGS = 0xff;  
		  PORTC.INTFLAGS = 0xff;
		  PORTE.INTFLAGS = 0xff;
  // ***********************************************************

sei(); 

}     // RETURN OF RPG ISR ??
     






//********************************************************************
// ISR CODE IS FOR BOTH PORTC INTERRUPTS && PORTE INTERRUPTS
// (Full Startup Sequence Controller SSC INTERFACE Signals)
// For the TOP LEVEL FSM operation (Not instrumenation related!)
// ---------------------------------------------------------
//  Description:
//    These signals drive state changes for the top level FSM
//    (finite state machine).
//
//    In this MINIMIZED version, the main triggers are the IGN
//    signal and the Charge-Active signal...  Both edges of these
//    signals are INTERRUPT driven.
//
//       IGN   : PC7 (1=On, 0=off)
//		 CHRG  : PA4 (1=On, 0=off)
//
//      Note... The contactor signal is NOT interrupt driven !
//      -----------------------
//        CONTACTOR: PE0 (1=Closed, 0=Open)
//
//******************************************************************

// PORT-C  &  PORT-E  VECTORS 
ISR_ALIAS(PORTC_PORT_vect, PORTE_PORT_vect)  
ISR (PORTE_PORT_vect)    // (Door, IGN, Cntctr, etc...)   
  {
	cli();

	// debounce delay
	_delay_ms(40);
		 
    // Entry location... Get current PORT values
	PORTC_image = PORTC.IN;  // get trigger bit values
	PORTE_image = PORTE.IN;  

	// Check the INPUT trigger signals, based on the current STATE, and determine
	// the appropriate NEXT_STATE
	switch (top_state_num) {
		case STANDBY_STATE:  //OLEDs plus NOT powered Up
			// Check if key inserted (KEY bit == 1)
			if ((PORTE_image & PIN2_bm) != 0) {   // KEY inserted = 1 
				top_state_num = WAKE1_STATE;      // Key out = 0
				standby_state_active_flag = 0;
				charge_cycle_active_flag = 0; // NOT a charge cycle
				evim_state_active_flag = 1;  // Restart/Warm reset
				}

			else if ((PORTE_image & PIN1_bm) == 0) {   // DOOR opened 
				pedal_lock_pwr_on(); // hold pedal-loc in locked position
				TCA0_init();  // start timout timer

				top_state_num = WAKE1_STATE;      // Key out = 0
				standby_state_active_flag = 0;
				charge_cycle_active_flag = 0; // NOT a charge cycle
				evim_state_active_flag = 1;  // Restart/Warm reset ... to WAKE1
				}

			break;

                                    // ----  IMORTANT   ----------------------------	
		case WAKE1_STATE:   // OLEDs power up, w/SoCH, but this code section
                                    // really DOES NOT RUN, as WAKE1 is a Puedo State
                                    // that is passed thru directly entering the 
                                    // EVIM_STATE, and main instrumentation mode
                                    // ---------------------------------------------	
					break;
		case IGNITION_STATE :
			   // Check if Front Contactor Closed Signal = '1'...
			   if ((PORTE_image & PIN0_bm) != 0) { // CONTACTOR CLOSED!
				   top_state_num = EVIM_STATE;
			   }
			   break;

			   			   
		case EVIM_STATE :
	         if ((!(PORTE_image & PIN2_bm)) && (!(PORTE_image & PIN1_bm)) 
			        && (charge_cycle_active_flag == 0) && (rpg_on_flag == 0)){
		
				  top_state_num = STANDBY_STATE;
				  standby_state_active_flag = 0;
				  charge_cycle_active_flag = 0; // NOT a charge cycle
				  evim_state_active_flag = 0;  // Restart/Warn reset

				 // Perform SOFTWARE Reset operation
				 _PROTECTED_WRITE(RSTCTRL.SWRR, PIN0_bm);

				//  ??? NECESSARY ??
				break;
			}
	
			// CHECK for change of MODE switch (On center upper dash)...
			new_dsp_mode_sw_val = PORTC.IN & PIN3_bm;
			// Check for display mode switch change...
			if ((new_dsp_mode_sw_val) != (dsp_mode_sw_val)) {
				dsp_mode_flag = dsp_mode_flag ^ PIN0_bm;
				mode_changed = 1;  // set active
				dsp_mode_sw_val = new_dsp_mode_sw_val;
				break;
				}
				
				
		default :   // Required???  Don't think so...
		            //  clear them all!
					PORTA.INTFLAGS = 0xff;  
					PORTB.INTFLAGS = 0xff;
					PORTC.INTFLAGS = 0xff;
					PORTE.INTFLAGS = 0xff;
					break;
		}  //END OF SWITCH STATEMENT


	PORTA.INTFLAGS = 0xff;    // clear them all!
	PORTB.INTFLAGS = 0xff;
	PORTC.INTFLAGS = 0xff;
	PORTE.INTFLAGS = 0xff;
    sei ();

}  // END ISR



// ***** ISR (PORTD-DEBUG-TRAP... PORTD = Analog Inputs, so NO Intrs)
// ***** ISR (PORTD-DEBUG-TRAP... PORTD = Analog Inputs, so NO Intrs)
ISR (PORTD_PORT_vect)
  {
	  while(1);     //  MAKE INTO A RETURN FOR DEPLOYED VERSIONS
  }
// ***** ISR END
  
  