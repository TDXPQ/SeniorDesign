################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../main_128_wSoCH_wTCoff_wRPGon_R3.c


PREPROCESSING_SRCS +=  \
../fclk_set_8mhz_asm.S


ASM_SRCS += 


OBJS +=  \
fclk_set_8mhz_asm.o \
main_128_wSoCH_wTCoff_wRPGon_R3.o

OBJS_AS_ARGS +=  \
fclk_set_8mhz_asm.o \
main_128_wSoCH_wTCoff_wRPGon_R3.o

C_DEPS +=  \
fclk_set_8mhz_asm.d \
main_128_wSoCH_wTCoff_wRPGon_R3.d

C_DEPS_AS_ARGS +=  \
fclk_set_8mhz_asm.d \
main_128_wSoCH_wTCoff_wRPGon_R3.d

OUTPUT_FILE_PATH +=WMOS_AVR_Code.elf

OUTPUT_FILE_PATH_AS_ARGS +=WMOS_AVR_Code.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler
./main_128_wSoCH_wTCoff_wRPGon_R3.o: .././main_128_wSoCH_wTCoff_wRPGon_R3.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\AVR-Dx_DFP\1.10.114\include" -I"../Dependencies"  -O0 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=avr128db64 -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\AVR-Dx_DFP\1.10.114\gcc\dev\avr128db64" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	




# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler
./fclk_set_8mhz_asm.o: .././fclk_set_8mhz_asm.S
	@echo Building file: $<
	@echo Invoking: AVR/GNU Assembler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=avr128db64 -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\AVR-Dx_DFP\1.10.114\gcc\dev\avr128db64" -I "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\AVR-Dx_DFP\1.10.114\include"  -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
	@echo Finished building: $<
	





ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="WMOS_AVR_Code.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=avr128db64 -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\AVR-Dx_DFP\1.10.114\gcc\dev\avr128db64"  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "WMOS_AVR_Code.elf" "WMOS_AVR_Code.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "WMOS_AVR_Code.elf" "WMOS_AVR_Code.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "WMOS_AVR_Code.elf" > "WMOS_AVR_Code.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "WMOS_AVR_Code.elf" "WMOS_AVR_Code.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "WMOS_AVR_Code.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "WMOS_AVR_Code.elf" "WMOS_AVR_Code.a" "WMOS_AVR_Code.hex" "WMOS_AVR_Code.lss" "WMOS_AVR_Code.eep" "WMOS_AVR_Code.map" "WMOS_AVR_Code.srec" "WMOS_AVR_Code.usersignatures"
	