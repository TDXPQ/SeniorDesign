// Globabl Variables for Wireless Remote
// Ask Scott where to put these
char command[50];
uint8_t cmd_index = 0;
char c;

// Wireless Remote function headers
void USART5_Init(void);
void USART5_sendChar(char c);
void USART5_sendString(char *str);
char USART5_readChar(void);
void executeCommand(char *command);

#include <stdio.h> // Needed libraries


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
		USART5_sendString((char*)pack_kwh_array);
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
		
		sprintf(temp, "%u%u.%u%u", accy133_tens, accy133_units, accy133_tenths, accy133_hundredths);
		USART5_sendString((char*)temp);
		USART5_sendChar('\n');
		
	}
	//default case is to send out instructions on RX pin, no change to LED
	else
	{
		
	}
}