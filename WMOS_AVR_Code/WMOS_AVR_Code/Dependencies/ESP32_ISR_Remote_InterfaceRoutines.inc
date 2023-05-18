
#include <stdio.h> // Needed libraries

 // Globabl Variables for Wireless Remote
char command[50];
uint8_t cmd_index = 0;
char c;

// Wireless Remote function headers
void USART5_Init(void);
void remoteInterface_Init(void);
void USART5_sendChar(char c);
void USART5_sendString(char *str);
char USART5_readChar(void);
void executeCommand(char *command);
void remoteInterface_Init(void);
void esp32_enable_relay(void);
void esp32_disable_relay(void);
void esp32_enable_threshold(void);
void esp32_disable_threshold(void);


ISR ( USART5_RXC_vect ){ //Interrupt the program when there is a new USART command
	cli();
	c = USART5_readChar(); // Read character from USAER
	
	// Keep reading characters until you get the entire command
	if(c != '\n' && c != '\r')
	{
		command[cmd_index++] = c;
		if(cmd_index > 50)
		{
			cmd_index = 0;
		}
	}
	if(c == '\n')
	{
		command[cmd_index] = '\0';
		cmd_index = 0;
		executeCommand(command); //When you get the entire command execute it
		
	}
	sei();
	
}

void USART5_Init(){

	PORTG.DIR &= ~PIN1_bm; //Enable PortC Pin 1 as the USART RX
	PORTG.DIR |= PIN0_bm; //Enable PORT C Pin 0 as the USART TX
	
	USART5.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;    // Turn on the transmission and reception circuitry
	
	USART5.BAUD = (uint16_t)USART_BAUD_RATE(115200); //Set the BAUD Rate
	
	USART5.CTRLA |= USART_RXCIE_bm; //Enable hardware interrupt
	sei ();	// Enable global interrupts
}

void remoteInterface_Init(void){
	/*
	Set up PortG pins for the enable and relay
	*/
	PORTG.DIRSET = (PIN2_bm | PIN3_bm | PIN4_bm); // Set Port G Pins 2,3,4 as output
	esp32_enable_relay();

	USART5_Init(); // Call USART5_Init()
}

void USART5_sendChar(char c)
{
	//function is hungup until status is ready
	while (!(USART5.STATUS & USART_DREIF_bm))
	{
		;
	}
	//send char c over TX pin
	USART5.TXDATAL = c;
}

void USART5_sendString(char *str)
{
	//repeated calls to send char for each char in the char array
	for(size_t i = 0; i < strlen(str); i++)
	{
		USART5_sendChar(str[i]);
	}
}

void esp32_enable_relay(void){
	PORTG.OUTSET |= (PIN2_bm | PIN4_bm); // Set Port G Pin 2 and 4 and high
}

void esp32_disable_relay(void){
	PORTG.OUTCLR |= (PIN2_bm | PIN4_bm); // Set Port G Pin 2 and 4 and low
}

void esp32_enable_threshold(void){
	PORTG.OUTCLR |= (PIN3_bm); // Threshold is enabled low
}

void esp32_disable_threshold(void){
	PORTG.OUTSET |= (PIN3_bm); //Threshold is disabled high
}

char USART5_readChar(void)
{
	//function is hungup until USART is ready according to flag
	while (!(USART5.STATUS & USART_RXCIF_bm))
	{
		;
	}
	//return the single 8 bit char recieved on RX
	return USART5.RXDATAL;
}

void executeCommand(char *command)
{
	
	char temp[30];

	//compare received command to ON, turn Led on if command matches
	if(strcmp(command, "a") == 0)
	{
		USART5_sendString((char*)pack_voltage_array);
		USART5_sendChar('\n');
	}
	else if (strcmp(command, "b") == 0)
	{
		USART5_sendString((char*)pack_current_array);
		USART5_sendChar('\n');
	}
	else if (strcmp(command, "c") == 0)
	{
		USART5_sendString((char*)pack_soc_array);
		USART5_sendChar('\n');
	}
	else if (strcmp(command, "d") == 0)
	{
		char charArray[17];

		for(size_t i=0; i < 16; i++){
			charArray[i] = (char)pack_kwh_array[i];
		}
		charArray[16] = '\0';

		USART5_sendString((char*)charArray);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "e") == 0)
	{
		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%u%u.%u%u", aux5_tens, aux5_units, aux5_tenths, aux5_hundredths);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "f") == 0)
	{
		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%u%u.%u%u", aux12_tens, aux12_units, aux12_tenths, aux12_hundredths);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "g") == 0)
	{
		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%u%u.%u%u", accy133_tens, accy133_units, accy133_tenths, accy133_hundredths);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "h") == 0)
	{
		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[5]); //Convert value to string to transmit
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "i") == 0)
	{

		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[4]);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "j") == 0)
	{

		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[3]);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "k") == 0)
	{

		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[2]);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "l") == 0)
	{

		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[1]);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "m") == 0)
	{

		memset(temp, 0, sizeof(temp)); // Reset the temp variable to send information
		
		sprintf(temp, "%hu", scaled_temps_array[0]);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	else if (strcmp(command, "z") == 0)
	{
		esp32_disable_relay(); // Disable power the the relay and ESP
		_delay_ms(30); //Delay
		esp32_enable_relay(); // Enable power to the relay and ESP
		
	}
	//default case is to send out instructions on RX pin, no change to LED
	else
	{
		
	}
}