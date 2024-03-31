

#include <LabMovEsp32.h>
#include <inttypes.h>
#include <pgmspace.h>

/* combine two 8 bit data's to form a 16 bit data */
//#define BME680_CONCAT_BYTES(msb, lsb)	((msb << 8) | lsb)


// parameters
int32_t _t_temp;
int32_t _t_pressure;
int32_t _t_humidity;


/**
 @brief Constructor
*/

BME680C::BME680C(int addr){
  // set device address
  device_addr = addr;
  Wire.begin(0,16,100000);
}

/**
 @brief Begin Device
 @retval true normaly done
 @retval false device error
*/
bool BME680C::begin() {
	uint8_t device = readByteI2c(BME680_CHIP_ID_REG);

	if (device == BME680_CHIP_ID) {
		writeI2c(BME680_RESET_REG, BME680_INITIATES_RESET_CMD);
		delay(100);
        readCalibrationParameter();
		return true;
	} 
	else {
    return false;
	}
}

/**
 * @brief setOversampling
 */
void BME680C::setParam(uint8_t osrs_temp, uint8_t osrs_press, uint8_t osrs_hum, uint8_t filter)
{
  _temp_osrs = osrs_temp;
  _hum_osrs = osrs_hum;
  _press_osrs = osrs_press;
  _iir_filter = filter;
}


/**
 * @brief Set Mode
 */
void BME680C::setMode(uint8_t mode)
{
	uint8_t ctrl_meas_val;
	uint8_t pow_mode = 0;

  // Check SLEEP_MODE
	do {
		ctrl_meas_val = readByteI2c(BME680_CTRL_MEAS_REG);
		/* Put to sleep before changing mode */
		pow_mode = ctrl_meas_val & BME680_MODE_MASK;

		if (pow_mode != BME680_SLEEP_MODE) {
			ctrl_meas_val = ctrl_meas_val & (~BME680_MODE_MASK); /* Set to sleep */
			writeI2c(BME680_CTRL_MEAS_REG, ctrl_meas_val);
			delay(50);
		}
	} while (pow_mode != BME680_SLEEP_MODE);

	/* set mode */
	if (mode != BME680_SLEEP_MODE) {
		ctrl_meas_val = ((ctrl_meas_val & (~BME680_MODE_MASK)) | mode);
		writeI2c(BME680_CTRL_MEAS_REG, ctrl_meas_val);
	}
}


/**
 * @brief read Sensor Value
 * @retval true: data found
 * @retval false: no data found
 */
bool BME680C::readSensors()
{
	// check status
	if ((readByteI2c(BME680_EAS_STATUS_REG)&BME680_EAS_STATUS_MEASURING)!=0){
		return false;
	};

	// set sleep mode
	setMode(BME680_SLEEP_MODE);
	// set sensor data
	writeI2c(BME680_CTRL_GAS0_REG, BME680_CTRL_GAS0_HEATER_OFF);
	writeI2c(BME680_CTRL_GAS1_REG, (readByteI2c(BME680_CTRL_GAS1_REG) & BME680_CTRL_RUN_GAS_MASK));

	uint8_t read_config = (readByteI2c(BME680_CONFIG_REG) & (~BME680_CONFIG_IIR_FILTER_MASK));
	read_config |= _iir_filter;
	writeI2c(BME680_CONFIG_REG, read_config);
	uint8_t read_mode = (readByteI2c(BME680_CTRL_MEAS_REG) & BME680_MODE_MASK);
	uint8_t ctrl_meas = read_mode
		| (_temp_osrs << BME680_OSRS_TEMP_BIT_SHIFT)
		| (_press_osrs << BME680_OSRS_PRESS_BIT_SHIFT);
	writeI2c(BME680_CTRL_MEAS_REG, ctrl_meas);
	writeI2c(BME680_CTRL_HUM_REG, _hum_osrs << BME680_OSRS_HUM_BIT_SHIFT);

	setMode(BME680_FORCED_MODE);
	// read delay time

	uint16_t duration;
	// Over Sampling List
	uint8_t dur_array[6] = { 0, 1, 2, 4, 8, 16 };

	duration = dur_array[_temp_osrs];    // temp
	duration += dur_array[_hum_osrs];    // humidity
	duration += dur_array[_press_osrs];  // pressure
	/* TPH measurement duration */
	duration = duration * 1963;
	duration += 4793;
	duration /= 1000;     /* Convert to ms */
	duration += 1;        /* Wake up duration of 1ms */
	// New data Check
	do{
		delay(duration * 2);
	}while ((readByteI2c(BME680_EAS_STATUS_REG)&BME680_EAS_STATUS_NEW_DATA)!=0);

	/* calculation sensor data */

	// temperature
	uint32_t rawData;
	int32_t _t_fine;
	int32_t var1 = 0, var2 = 0, var3 = 0, var4 = 0, var5 = 0, var6 = 0;

	readI2c(BME680_TEMP_MSB_REG, 3);
	rawData = (((uint32_t) raw_data[0] << 16 | (uint32_t) raw_data[1] << 8 | raw_data[2]) >> 4);

	// set temperature in DegC
	var1 = ((int32_t)rawData >> 3) - ((int32_t)BME680_TEM_PAR_1 << 1);
	var2 = (var1 * (int32_t)BME680_TEM_PAR_2) >> 11;
	var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t) BME680_TEM_PAR_3 << 4)) >> 14;
	_t_fine = var2 + var3;
	_t_temp = (_t_fine * 5 + 128) >> 8;

	// Pressure
	readI2c(BME680_PRESS_MSB_REG, 3);
	rawData = (uint32_t) (((uint32_t) raw_data[0] << 16
                 | (uint32_t) raw_data[1] << 8 | raw_data[2]) >> 4);

	// set Pascal
	int32_t pascal = 0;
	var1 = (((int32_t) _t_fine) >> 1) - 64000;
	var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t) BME680_PRESS_PAR_6) >> 2;
	var2 = var2 + ((var1 * (int32_t)BME680_PRESS_PAR_5) << 1);
	var2 = (var2 >> 2) + ((int32_t) BME680_PRESS_PAR_4 << 16);
	var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t) BME680_PRESS_PAR_3 << 5)) >> 3)
         + (((int32_t) BME680_PRESS_PAR_2 * var1) >> 1);
	var1 = var1 >> 18;
	var1 = ((32768 + var1) * (int32_t) BME680_PRESS_PAR_1) >> 15;
	pascal = 1048576 - rawData;
	pascal = (int32_t)((pascal - (var2 >> 12)) * ((uint32_t)3125));

	// check over flow
	if(pascal >= 0x40000000){
		pascal = (( pascal / (uint32_t) var1) << 1);
	} else {
		pascal = ((pascal << 1) / (uint32_t) var1);
	}

	var1 = ((int32_t) BME680_PRESS_PAR_9 * (int32_t) (((pascal >> 3)
			* (pascal >> 3)) >> 13)) >> 12;
	var2 = ((int32_t)(pascal >> 2) * (int32_t) BME680_PRESS_PAR_8) >> 13;
	var3 = ((int32_t)(pascal >> 8) * (int32_t)(pascal >> 8)
         * (int32_t)(pascal >> 8) * (int32_t)BME680_PRESS_PAR_10) >> 17;
	pascal = (int32_t)(pascal) + ((var1 + var2 + var3
           + ((int32_t)BME680_PRESS_PAR_7 << 7)) >> 4);
	_t_pressure = pascal;

	// set humidity
	readI2c(BME680_HUM_MSB_REG, 2);
	rawData = (((uint16_t) raw_data[0] << 8 | raw_data[1]) );

	int32_t temperature;
	int32_t humidity;

	temperature = (((int32_t) _t_fine * 5) + 128) >> 8;
	var1 = (int32_t) rawData  - ((int32_t) ((int32_t)BME680_HUM_PAR_1 << 4)) - (((temperature * (int32_t) BME680_HUM_PAR_3) / ((int32_t)100)) >> 1);
	var2 = ((int32_t)BME680_HUM_PAR_2 * (((temperature * (int32_t)BME680_HUM_PAR_4)
			 / ((int32_t)100)) + (((temperature * ((temperature * (int32_t)BME680_HUM_PAR_5)
			 / ((int32_t)100))) >> 6) / ((int32_t)100)) + (int32_t)(1 << 14))) >> 10;
	var3 = var1 * var2;
	var4 = ((((int32_t)BME680_HUM_PAR_6) << 7) + ((temperature * (int32_t) BME680_HUM_PAR_7)
		     / ((int32_t)100))) >> 4;
	var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
	var6 = (var4 * var5) >> 1;

	humidity = (var3 + var6) >> 12;

	// check for over- and under-flow
	if (humidity > 102400) {
	humidity = 102400;
	} else if(humidity < 0) {
	humidity = 0;
	}
	_t_humidity = humidity;

	return true;
}

void BME680C::readCalibrationParameter()
{
    // BME680 compensation parameters

    int8_t rslt;
    uint8_t coeff_array[BME680_COEFF_SIZE] = { 0 };
    uint8_t temp_var = 0; /* Temporary variable */
    readI2c(BME680_CALIB1_START_REG, 25, coeff_array);
    /* Append the second half in the same array */
    readI2c(BME680_CALIB2_START_REG, 16, &coeff_array[25]);

    /* Temperature related coefficients */
    BME680_TEM_PAR_1 = (uint16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_T1_MSB_REG],
        coeff_array[BME680_T1_LSB_REG]));
    BME680_TEM_PAR_2 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_T2_MSB_REG],
        coeff_array[BME680_T2_LSB_REG]));
    BME680_TEM_PAR_3 = (int8_t)(coeff_array[BME680_T3_REG]);

    /* Pressure related coefficients */
    BME680_PRESS_PAR_1 = (uint16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P1_MSB_REG],
        coeff_array[BME680_P1_LSB_REG]));
    BME680_PRESS_PAR_2 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P2_MSB_REG],
        coeff_array[BME680_P2_LSB_REG]));
    BME680_PRESS_PAR_3 = (int8_t)coeff_array[BME680_P3_REG];
    BME680_PRESS_PAR_4 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P4_MSB_REG],
        coeff_array[BME680_P4_LSB_REG]));
    BME680_PRESS_PAR_5 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P5_MSB_REG],
        coeff_array[BME680_P5_LSB_REG]));
    BME680_PRESS_PAR_6 = (int8_t)(coeff_array[BME680_P6_REG]);
    BME680_PRESS_PAR_7 = (int8_t)(coeff_array[BME680_P7_REG]);
    BME680_PRESS_PAR_8 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P8_MSB_REG],
        coeff_array[BME680_P8_LSB_REG]));
    BME680_PRESS_PAR_9 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[BME680_P9_MSB_REG],
        coeff_array[BME680_P9_LSB_REG]));
    BME680_PRESS_PAR_10 = (uint8_t)(coeff_array[BME680_P10_REG]);

    /* Humidity related coefficients */
    BME680_HUM_PAR_1 = (uint16_t)(((uint16_t)coeff_array[BME680_H1_MSB_REG] << BME680_HUM_REG_SHIFT_VAL)
        | (coeff_array[BME680_H1_LSB_REG] & BME680_BIT_H1_DATA_MSK));
    BME680_HUM_PAR_2 = (uint16_t)(((uint16_t)coeff_array[BME680_H2_MSB_REG] << BME680_HUM_REG_SHIFT_VAL)
        | ((coeff_array[BME680_H2_LSB_REG]) >> BME680_HUM_REG_SHIFT_VAL));
    BME680_HUM_PAR_3 = (int8_t)coeff_array[BME680_H3_REG];
    BME680_HUM_PAR_4 = (int8_t)coeff_array[BME680_H4_REG];
    BME680_HUM_PAR_5 = (int8_t)coeff_array[BME680_H5_REG];
    BME680_HUM_PAR_6 = (uint8_t)coeff_array[BME680_H6_REG];
    BME680_HUM_PAR_7 = (int8_t)coeff_array[BME680_H7_REG];

}
/*
 * @brief read Temperature
 * @return uint32_t : Temperature (Degree Celsius)
 */
float BME680C::SensorValor(uint8_t modos)
{
	switch (modos)
	{
	case 0:
		return (float)_t_temp / 100;
	case 1:
		return (float)_t_pressure;
	case 2:
		return (float)_t_humidity / 1024;
	case 3:
		return (float)145366.45f * (1.0f - powf((_t_pressure / 100 / 1013.25f), 0.190284f)) * 0.3048;
	default:
		break;
	}
	
}

/**
 * @brief Write I2C Data
 * @param [in] register_addr : Write Register Address
 * @param [in] value  : Write Data
 */
void BME680C::writeI2c(uint8_t register_addr, uint8_t value) {
  Wire.beginTransmission(device_addr);
  Wire.write(register_addr);
  Wire.write(value);
  Wire.endTransmission();
}

/**
 * @brief Read I2C Data
 * @param [in] register_addr : register address
 * @param [in] num   : Data Length
 * @param [out] *buf : Read Data
 */
void BME680C::readI2c(uint8_t register_addr, uint8_t num) {
	Wire.beginTransmission(device_addr);
	Wire.write(register_addr);
	Wire.endTransmission();
	//Wire.beginTransmission(DEVICE_ADDR);
	Wire.requestFrom((int)device_addr, (int)num);
	int i = 0;
	while (Wire.available())
	{
		raw_data[i] = Wire.read();
		i++;
	}
}

void BME680C::readI2c(uint8_t register_addr, uint8_t num, uint8_t* buf) {
    Wire.beginTransmission(device_addr);
    Wire.write(register_addr);
    Wire.endTransmission();
    Wire.requestFrom((int)device_addr, (int)num);

    int i = 0;
    while (Wire.available())
    {
        buf[i] = Wire.read();
        i++;
    }
}

/**
 * @brief Read I2C Byte
 * @param [in] register_addr : register address
 * @return uint8_t : Read byte
 */
uint8_t BME680C::readByteI2c(uint8_t register_addr) {
  Wire.beginTransmission(device_addr);
  Wire.write(register_addr);
  Wire.endTransmission();
  Wire.requestFrom(device_addr, 1);
  uint8_t value = Wire.read();
  return value;
}


/////////////////////////////////////////////  lcd //////////////////////////////////////////////////


#define printIIC(args)	Wire.write(args)
inline size_t LiquidCrystal_I2C::write(uint8_t value) {
	send(value, Rs);
	return 1;
}

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows)
{
	_Addrr = lcd_Addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_backlightval = LCD_BACKLIGHT;
}

void LiquidCrystal_I2C::init() {
	init_priv();
}

void LiquidCrystal_I2C::init_priv()
{
	//Wire.begin(0, 16, 100000);
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	begin(_cols, _rows);
}

void LiquidCrystal_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50);

	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

	// second try
	write4bits(0x03 << 4);
	delayMicroseconds(4500); // wait min 4.1ms

	// third go!
	write4bits(0x03 << 4);
	delayMicroseconds(150);

	// finally, set to 4-bit interface
	write4bits(0x02 << 4);


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	// clear it off
	clear();

	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	home();

}

/********** high level commands, for the user! */
void LiquidCrystal_I2C::clear() {
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal_I2C::home() {
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row) {
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _numlines) {
		row = _numlines - 1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i = 0; i < 8; i++) {
		write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LiquidCrystal_I2C::noBacklight(void) {
	_backlightval = LCD_NOBACKLIGHT;
	expanderWrite(0);
}

void LiquidCrystal_I2C::backlight(void) {
	_backlightval = LCD_BACKLIGHT;
	expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal_I2C::command(uint8_t value) {
	send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void LiquidCrystal_I2C::send(uint8_t value, uint8_t mode) {
	uint8_t highnib = value & 0xf0;
	uint8_t lownib = (value << 4) & 0xf0;
	write4bits((highnib) | mode);
	write4bits((lownib) | mode);
}

void LiquidCrystal_I2C::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void LiquidCrystal_I2C::expanderWrite(uint8_t _data) {
	Wire.beginTransmission(_Addrr);
	printIIC((int)(_data) | _backlightval);
	Wire.endTransmission();
}

void LiquidCrystal_I2C::pulseEnable(uint8_t _data) {
	expanderWrite(_data | En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns

	expanderWrite(_data & ~En);	// En low
	delayMicroseconds(50);		// commands need > 37us to settle
}


// Alias functions

void LiquidCrystal_I2C::cursor_on() {
	cursor();
}

void LiquidCrystal_I2C::cursor_off() {
	noCursor();
}

void LiquidCrystal_I2C::blink_on() {
	blink();
}

void LiquidCrystal_I2C::blink_off() {
	noBlink();
}

void LiquidCrystal_I2C::load_custom_character(uint8_t char_num, uint8_t* rows) {
	createChar(char_num, rows);
}

void LiquidCrystal_I2C::setBacklight(uint8_t new_val) {
	if (new_val) {
		backlight();		// turn backlight on
	}
	else {
		noBacklight();		// turn backlight off
	}
}

void LiquidCrystal_I2C::printstr(const char c[]) {
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}


// unsupported API functions
void LiquidCrystal_I2C::off() {}
void LiquidCrystal_I2C::on() {}
void LiquidCrystal_I2C::setDelay(int cmdDelay, int charDelay) {}
uint8_t LiquidCrystal_I2C::status() { return 0; }
uint8_t LiquidCrystal_I2C::keypad() { return 0; }
uint8_t LiquidCrystal_I2C::init_bargraph(uint8_t graphtype) { return 0; }
void LiquidCrystal_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end) {}
void LiquidCrystal_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_row_end) {}
void LiquidCrystal_I2C::setContrast(uint8_t new_val) {}



/////////////////////////////////////////////  RTC    //////////////////////////////////////////////////



/**************************************************************************/
/*!
    @brief Write value to register.
    @param reg register address
    @param val value to write
*/
/**************************************************************************/
void RTC_I2C::write_register(uint8_t reg, uint8_t val) {
    uint8_t buffer[2] = { reg, val };
    i2c_dev->write(buffer, 2);
}

/**************************************************************************/
/*!
    @brief Read value from register.
    @param reg register address
    @return value of register
*/
/**************************************************************************/
uint8_t RTC_I2C::read_register(uint8_t reg) {
    uint8_t buffer[1];
    i2c_dev->write(&reg, 1);
    i2c_dev->read(buffer, 1);
    return buffer[0];
}

/**************************************************************************/
// utility code, some of this could be exposed in the DateTime API if needed
/**************************************************************************/

/**
  Number of days in each month, from January to November. December is not
  needed. Omitting it avoids an incompatibility with Paul Stoffregen's Time
  library. C.f. https://github.com/adafruit/RTClib/issues/114
*/
const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30 };

/**************************************************************************/
/*!
    @brief  Given a date, return number of days since 2000/01/01,
            valid for 2000--2099
    @param y Year
    @param m Month
    @param d Day
    @return Number of days
*/
/**************************************************************************/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000U)
        y -= 2000U;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

/**************************************************************************/
/*!
    @brief  Given a number of days, hours, minutes, and seconds, return the
   total seconds
    @param days Days
    @param h Hours
    @param m Minutes
    @param s Seconds
    @return Number of seconds total
*/
/**************************************************************************/
static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24UL + h) * 60 + m) * 60 + s;
}

/**************************************************************************/
/*!
    @brief  Constructor from
        [Unix time](https://en.wikipedia.org/wiki/Unix_time).

    This builds a DateTime from an integer specifying the number of seconds
    elapsed since the epoch: 1970-01-01 00:00:00. This number is analogous
    to Unix time, with two small differences:

     - The Unix epoch is specified to be at 00:00:00
       [UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time),
       whereas this class has no notion of time zones. The epoch used in
       this class is then at 00:00:00 on whatever time zone the user chooses
       to use, ignoring changes in DST.

     - Unix time is conventionally represented with signed numbers, whereas
       this constructor takes an unsigned argument. Because of this, it does
       _not_ suffer from the
       [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem).

    If called without argument, it returns the earliest time representable
    by this class: 2000-01-01 00:00:00.

    @see The `unixtime()` method is the converse of this constructor.

    @param t Time elapsed in seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
DateTime::DateTime(uint32_t t) {
    t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0;; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365U + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; m < 12; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

/**************************************************************************/
/*!
    @brief  Constructor from (year, month, day, hour, minute, second).
    @warning If the provided parameters are not valid (e.g. 31 February),
           the constructed DateTime will be invalid.
    @see   The `isValid()` method can be used to test whether the
           constructed DateTime is valid.
    @param year Either the full year (range: 2000--2099) or the offset from
        year 2000 (range: 0--99).
    @param month Month number (1--12).
    @param day Day of the month (1--31).
    @param hour,min,sec Hour (0--23), minute (0--59) and second (0--59).
*/
/**************************************************************************/
DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
    uint8_t min, uint8_t sec) {
    if (year >= 2000U)
        year -= 2000U;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

/**************************************************************************/
/*!
    @brief  Copy constructor.
    @param copy DateTime to copy.
*/
/**************************************************************************/
DateTime::DateTime(const DateTime& copy)
    : yOff(copy.yOff), m(copy.m), d(copy.d), hh(copy.hh), mm(copy.mm),
    ss(copy.ss) {}

/**************************************************************************/
/*!
    @brief  Convert a string containing two digits to uint8_t, e.g. "09" returns
   9
    @param p Pointer to a string containing two digits
*/
/**************************************************************************/
static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

/**************************************************************************/
/*!
    @brief  Constructor for generating the build time.

    This constructor expects its parameters to be strings in the format
    generated by the compiler's preprocessor macros `__DATE__` and
    `__TIME__`. Usage:

    ```
    DateTime buildTime(__DATE__, __TIME__);
    ```

    @note The `F()` macro can be used to reduce the RAM footprint, see
        the next constructor.

    @param date Date string, e.g. "Apr 16 2020".
    @param time Time string, e.g. "18:34:56".
*/
/**************************************************************************/
DateTime::DateTime(const char* date, const char* time) {
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0]) {
    case 'J':
        m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7);
        break;
    case 'F':
        m = 2;
        break;
    case 'A':
        m = date[2] == 'r' ? 4 : 8;
        break;
    case 'M':
        m = date[2] == 'r' ? 3 : 5;
        break;
    case 'S':
        m = 9;
        break;
    case 'O':
        m = 10;
        break;
    case 'N':
        m = 11;
        break;
    case 'D':
        m = 12;
        break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

/**************************************************************************/
/*!
    @brief  Memory friendly constructor for generating the build time.

    This version is intended to save RAM by keeping the date and time
    strings in program memory. Use it with the `F()` macro:

    ```
    DateTime buildTime(F(__DATE__), F(__TIME__));
    ```

    @param date Date PROGMEM string, e.g. F("Apr 16 2020").
    @param time Time PROGMEM string, e.g. F("18:34:56").
*/
/**************************************************************************/
DateTime::DateTime(const __FlashStringHelper* date,
    const __FlashStringHelper* time) {
    char buff[11];
    memcpy_P(buff, date, 11);
    yOff = conv2d(buff + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (buff[0]) {
    case 'J':
        m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7);
        break;
    case 'F':
        m = 2;
        break;
    case 'A':
        m = buff[2] == 'r' ? 4 : 8;
        break;
    case 'M':
        m = buff[2] == 'r' ? 3 : 5;
        break;
    case 'S':
        m = 9;
        break;
    case 'O':
        m = 10;
        break;
    case 'N':
        m = 11;
        break;
    case 'D':
        m = 12;
        break;
    }
    d = conv2d(buff + 4);
    memcpy_P(buff, time, 8);
    hh = conv2d(buff);
    mm = conv2d(buff + 3);
    ss = conv2d(buff + 6);
}

/**************************************************************************/
/*!
    @brief  Constructor for creating a DateTime from an ISO8601 date string.

    This constructor expects its parameters to be a string in the
    https://en.wikipedia.org/wiki/ISO_8601 format, e.g:

    "2020-06-25T15:29:37"

    Usage:

    ```
    DateTime dt("2020-06-25T15:29:37");
    ```

    @note The year must be > 2000, as only the yOff is considered.

    @param iso8601dateTime
           A dateTime string in iso8601 format,
           e.g. "2020-06-25T15:29:37".

*/
/**************************************************************************/
DateTime::DateTime(const char* iso8601dateTime) {
    char ref[] = "2000-01-01T00:00:00";
    memcpy(ref, iso8601dateTime, min(strlen(ref), strlen(iso8601dateTime)));
    yOff = conv2d(ref + 2);
    m = conv2d(ref + 5);
    d = conv2d(ref + 8);
    hh = conv2d(ref + 11);
    mm = conv2d(ref + 14);
    ss = conv2d(ref + 17);
}

/**************************************************************************/
/*!
    @brief  Check whether this DateTime is valid.
    @return true if valid, false if not.
*/
/**************************************************************************/
bool DateTime::isValid() const {
    if (yOff >= 100)
        return false;
    DateTime other(unixtime());
    return yOff == other.yOff && m == other.m && d == other.d && hh == other.hh &&
        mm == other.mm && ss == other.ss;
}

/**************************************************************************/
/*!
    @brief  Writes the DateTime as a string in a user-defined format.

    The _buffer_ parameter should be initialized by the caller with a string
    specifying the requested format. This format string may contain any of
    the following specifiers:

    | specifier | output                                                 |
    |-----------|--------------------------------------------------------|
    | YYYY      | the year as a 4-digit number (2000--2099)              |
    | YY        | the year as a 2-digit number (00--99)                  |
    | MM        | the month as a 2-digit number (01--12)                 |
    | MMM       | the abbreviated English month name ("Jan"--"Dec")      |
    | DD        | the day as a 2-digit number (01--31)                   |
    | DDD       | the abbreviated English day of the week ("Mon"--"Sun") |
    | AP        | either "AM" or "PM"                                    |
    | ap        | either "am" or "pm"                                    |
    | hh        | the hour as a 2-digit number (00--23 or 01--12)        |
    | mm        | the minute as a 2-digit number (00--59)                |
    | ss        | the second as a 2-digit number (00--59)                |

    If either "AP" or "ap" is used, the "hh" specifier uses 12-hour mode
    (range: 01--12). Otherwise it works in 24-hour mode (range: 00--23).

    The specifiers within _buffer_ will be overwritten with the appropriate
    values from the DateTime. Any characters not belonging to one of the
    above specifiers are left as-is.

    __Example__: The format "DDD, DD MMM YYYY hh:mm:ss" generates an output
    of the form "Thu, 16 Apr 2020 18:34:56.

    @see The `timestamp()` method provides similar functionnality, but it
        returns a `String` object and supports a limited choice of
        predefined formats.

    @param[in,out] buffer Array of `char` for holding the format description
        and the formatted DateTime. Before calling this method, the buffer
        should be initialized by the user with the format string. The method
        will overwrite the buffer with the formatted date and/or time.

    @return A pointer to the provided buffer. This is returned for
        convenience, in order to enable idioms such as
        `Serial.println(now.toString(buffer));`
*/
/**************************************************************************/

char* DateTime::toString(char* buffer) const {
    uint8_t apTag =
        (strstr(buffer, "ap") != nullptr) || (strstr(buffer, "AP") != nullptr);
    uint8_t hourReformatted = 0, isPM = false;
    if (apTag) {     // 12 Hour Mode
        if (hh == 0) { // midnight
            isPM = false;
            hourReformatted = 12;
        }
        else if (hh == 12) { // noon
            isPM = true;
            hourReformatted = 12;
        }
        else if (hh < 12) { // morning
            isPM = false;
            hourReformatted = hh;
        }
        else { // 1 o'clock or after
            isPM = true;
            hourReformatted = hh - 12;
        }
    }

    for (size_t i = 0; i < strlen(buffer) - 1; i++) {
        if (buffer[i] == 'h' && buffer[i + 1] == 'h') {
            if (!apTag) { // 24 Hour Mode
                buffer[i] = '0' + hh / 10;
                buffer[i + 1] = '0' + hh % 10;
            }
            else { // 12 Hour Mode
                buffer[i] = '0' + hourReformatted / 10;
                buffer[i + 1] = '0' + hourReformatted % 10;
            }
        }
        if (buffer[i] == 'm' && buffer[i + 1] == 'm') {
            buffer[i] = '0' + mm / 10;
            buffer[i + 1] = '0' + mm % 10;
        }
        if (buffer[i] == 's' && buffer[i + 1] == 's') {
            buffer[i] = '0' + ss / 10;
            buffer[i + 1] = '0' + ss % 10;
        }
        if (buffer[i] == 'D' && buffer[i + 1] == 'D' && buffer[i + 2] == 'D') {
            static PROGMEM const char day_names[] = "SunMonTueWedThuFriSat";
            const char* p = &day_names[3 * dayOfTheWeek()];
            buffer[i] = pgm_read_byte(p);
            buffer[i + 1] = pgm_read_byte(p + 1);
            buffer[i + 2] = pgm_read_byte(p + 2);
        }
        else if (buffer[i] == 'D' && buffer[i + 1] == 'D') {
            buffer[i] = '0' + d / 10;
            buffer[i + 1] = '0' + d % 10;
        }
        if (buffer[i] == 'M' && buffer[i + 1] == 'M' && buffer[i + 2] == 'M') {
            static PROGMEM const char month_names[] =
                "JanFebMarAprMayJunJulAugSepOctNovDec";
            const char* p = &month_names[3 * (m - 1)];
            buffer[i] = pgm_read_byte(p);
            buffer[i + 1] = pgm_read_byte(p + 1);
            buffer[i + 2] = pgm_read_byte(p + 2);
        }
        else if (buffer[i] == 'M' && buffer[i + 1] == 'M') {
            buffer[i] = '0' + m / 10;
            buffer[i + 1] = '0' + m % 10;
        }
        if (buffer[i] == 'Y' && buffer[i + 1] == 'Y' && buffer[i + 2] == 'Y' &&
            buffer[i + 3] == 'Y') {
            buffer[i] = '2';
            buffer[i + 1] = '0';
            buffer[i + 2] = '0' + (yOff / 10) % 10;
            buffer[i + 3] = '0' + yOff % 10;
        }
        else if (buffer[i] == 'Y' && buffer[i + 1] == 'Y') {
            buffer[i] = '0' + (yOff / 10) % 10;
            buffer[i + 1] = '0' + yOff % 10;
        }
        if (buffer[i] == 'A' && buffer[i + 1] == 'P') {
            if (isPM) {
                buffer[i] = 'P';
                buffer[i + 1] = 'M';
            }
            else {
                buffer[i] = 'A';
                buffer[i + 1] = 'M';
            }
        }
        else if (buffer[i] == 'a' && buffer[i + 1] == 'p') {
            if (isPM) {
                buffer[i] = 'p';
                buffer[i + 1] = 'm';
            }
            else {
                buffer[i] = 'a';
                buffer[i + 1] = 'm';
            }
        }
    }
    return buffer;
}

/**************************************************************************/
/*!
      @brief  Return the hour in 12-hour format.
      @return Hour (1--12).
*/
/**************************************************************************/
uint8_t DateTime::twelveHour() const {
    if (hh == 0 || hh == 12) { // midnight or noon
        return 12;
    }
    else if (hh > 12) { // 1 o'clock or later
        return hh - 12;
    }
    else { // morning
        return hh;
    }
}

/**************************************************************************/
/*!
    @brief  Return the day of the week.
    @return Day of week as an integer from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
uint8_t DateTime::dayOfTheWeek() const {
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

/**************************************************************************/
/*!
    @brief  Return Unix time: seconds since 1 Jan 1970.

    @see The `DateTime::DateTime(uint32_t)` constructor is the converse of
        this method.

    @return Number of seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::unixtime(void) const {
    uint32_t t;
    uint16_t days = date2days(yOff, m, d);
    t = time2ulong(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000

    return t;
}

/**************************************************************************/
/*!
    @brief  Convert the DateTime to seconds since 1 Jan 2000

    The result can be converted back to a DateTime with:

    ```cpp
    DateTime(SECONDS_FROM_1970_TO_2000 + value)
    ```

    @return Number of seconds since 2000-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::secondstime(void) const {
    uint32_t t;
    uint16_t days = date2days(yOff, m, d);
    t = time2ulong(days, hh, mm, ss);
    return t;
}

/**************************************************************************/
/*!
    @brief  Add a TimeSpan to the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span added to it.
*/
/**************************************************************************/
DateTime DateTime::operator+(const TimeSpan& span) const {
    return DateTime(unixtime() + span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan from the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span subtracted from it.
*/
/**************************************************************************/
DateTime DateTime::operator-(const TimeSpan& span) const {
    return DateTime(unixtime() - span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract one DateTime from another

    @note Since a TimeSpan cannot be negative, the subtracted DateTime
        should be less (earlier) than or equal to the one it is
        subtracted from.

    @param right The DateTime object to subtract from self (the left object)
    @return TimeSpan of the difference between DateTimes.
*/
/**************************************************************************/
TimeSpan DateTime::operator-(const DateTime& right) const {
    return TimeSpan(unixtime() - right.unixtime());
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if one DateTime is less (earlier) than another.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if the left DateTime is earlier than the right one,
        false otherwise.
*/
/**************************************************************************/
bool DateTime::operator<(const DateTime& right) const {
    return (yOff + 2000U < right.year() ||
        (yOff + 2000U == right.year() &&
            (m < right.month() ||
                (m == right.month() &&
                    (d < right.day() ||
                        (d == right.day() &&
                            (hh < right.hour() ||
                                (hh == right.hour() &&
                                    (mm < right.minute() ||
                                        (mm == right.minute() && ss < right.second()))))))))));
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if two DateTime objects are equal.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if both DateTime objects are the same, false otherwise.
*/
/**************************************************************************/
bool DateTime::operator==(const DateTime& right) const {
    return (right.year() == yOff + 2000U && right.month() == m &&
        right.day() == d && right.hour() == hh && right.minute() == mm &&
        right.second() == ss);
}

/**************************************************************************/
/*!
    @brief  Return a ISO 8601 timestamp as a `String` object.

    The generated timestamp conforms to one of the predefined, ISO
    8601-compatible formats for representing the date (if _opt_ is
    `TIMESTAMP_DATE`), the time (`TIMESTAMP_TIME`), or both
    (`TIMESTAMP_FULL`).

    @see The `toString()` method provides more general string formatting.

    @param opt Format of the timestamp
    @return Timestamp string, e.g. "2020-04-16T18:34:56".
*/
/**************************************************************************/
String DateTime::timestamp(timestampOpt opt) const {
    char buffer[25]; // large enough for any DateTime, including invalid ones

    // Generate timestamp according to opt
    switch (opt) {
    case TIMESTAMP_TIME:
        // Only time
        sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
        break;
    case TIMESTAMP_DATE:
        // Only date
        sprintf(buffer, "%u-%02d-%02d", 2000U + yOff, m, d);
        break;
    default:
        // Full
        sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOff, m, d, hh, mm,
            ss);
    }
    return String(buffer);
}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object in seconds
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object using a number of
   days/hours/minutes/seconds e.g. Make a TimeSpan of 3 hours and 45 minutes:
   new TimeSpan(0, 3, 45, 0);
    @param days Number of days
    @param hours Number of hours
    @param minutes Number of minutes
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
    : _seconds((int32_t)days * 86400L + (int32_t)hours * 3600 +
        (int32_t)minutes * 60 + seconds) {}

/**************************************************************************/
/*!
    @brief  Copy constructor, make a new TimeSpan using an existing one
    @param copy The TimeSpan to copy
*/
/**************************************************************************/
TimeSpan::TimeSpan(const TimeSpan& copy) : _seconds(copy._seconds) {}

/**************************************************************************/
/*!
    @brief  Add two TimeSpans
    @param right TimeSpan to add
    @return New TimeSpan object, sum of left and right
*/
/**************************************************************************/
TimeSpan TimeSpan::operator+(const TimeSpan& right) const {
    return TimeSpan(_seconds + right._seconds);
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan
    @param right TimeSpan to subtract
    @return New TimeSpan object, right subtracted from left
*/
/**************************************************************************/
TimeSpan TimeSpan::operator-(const TimeSpan& right) const {
    return TimeSpan(_seconds - right._seconds);
}



#define DS3231_ADDRESS 0x68   ///< I2C address for DS3231
#define DS3231_TIME 0x00      ///< Time register
#define DS3231_ALARM1 0x07    ///< Alarm 1 register
#define DS3231_ALARM2 0x0B    ///< Alarm 2 register
#define DS3231_CONTROL 0x0E   ///< Control register
#define DS3231_STATUSREG 0x0F ///< Status register
#define DS3231_TEMPERATUREREG                                                  \
  0x11 ///< Temperature register (high byte - low byte is at 0x12), 10-bit
///< temperature value

/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @param  wireInstance pointer to the I2C bus
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/
bool RTC_DS3231::begin() {
    if (i2c_dev)
        delete i2c_dev;
    i2c_dev = new Adafruit_I2CDevice(DS3231_ADDRESS);
    if (!i2c_dev->begin())
        return false;
    return true;
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
   stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
   running
*/
/**************************************************************************/
bool RTC_DS3231::lostPower(void) {
    return read_register(DS3231_STATUSREG) >> 7;
}

/**************************************************************************/
/*!
    @brief  Set the date and flip the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
*/
/**************************************************************************/
void RTC_DS3231::adjust(const DateTime& dt) {
    uint8_t buffer[8] = { DS3231_TIME,
                         bin2bcd(dt.second()),
                         bin2bcd(dt.minute()),
                         bin2bcd(dt.hour()),
                         bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
                         bin2bcd(dt.day()),
                         bin2bcd(dt.month()),
                         bin2bcd(dt.year() - 2000U) };
    i2c_dev->write(buffer, 8);

    uint8_t statreg = read_register(DS3231_STATUSREG);
    statreg &= ~0x80; // flip OSF bit
    write_register(DS3231_STATUSREG, statreg);
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime RTC_DS3231::now() {
    uint8_t buffer[7];
    buffer[0] = 0;
    i2c_dev->write_then_read(buffer, 1, buffer, 7);

    return DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5] & 0x7F),
        bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
        bcd2bin(buffer[0] & 0x7F));
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
Ds3231SqwPinMode RTC_DS3231::readSqwPinMode() {
    int mode;
    mode = read_register(DS3231_CONTROL) & 0x1C;
    if (mode & 0x04)
        mode = DS3231_OFF;
    return static_cast<Ds3231SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
void RTC_DS3231::writeSqwPinMode(Ds3231SqwPinMode mode) {
    uint8_t ctrl = read_register(DS3231_CONTROL);

    ctrl &= ~0x04; // turn off INTCON
    ctrl &= ~0x18; // set freq bits to 0

    write_register(DS3231_CONTROL, ctrl | mode);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float RTC_DS3231::getTemperature() {
    uint8_t buffer[2] = { DS3231_TEMPERATUREREG, 0 };
    i2c_dev->write_then_read(buffer, 1, buffer, 2);
    return (float)buffer[0] + (buffer[1] >> 6) * 0.25f;
}

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode) {
    uint8_t ctrl = read_register(DS3231_CONTROL);
    if (!(ctrl & 0x04)) {
        return false;
    }

    uint8_t A1M1 = (alarm_mode & 0x01) << 7; // Seconds bit 7.
    uint8_t A1M2 = (alarm_mode & 0x02) << 6; // Minutes bit 7.
    uint8_t A1M3 = (alarm_mode & 0x04) << 5; // Hour bit 7.
    uint8_t A1M4 = (alarm_mode & 0x08) << 4; // Day/Date bit 7.
    uint8_t DY_DT = (alarm_mode & 0x10)
        << 2; // Day/Date bit 6. Date when 0, day of week when 1.
    uint8_t day = (DY_DT) ? dowToDS3231(dt.dayOfTheWeek()) : dt.day();

    uint8_t buffer[5] = { DS3231_ALARM1, uint8_t(bin2bcd(dt.second()) | A1M1),
                         uint8_t(bin2bcd(dt.minute()) | A1M2),
                         uint8_t(bin2bcd(dt.hour()) | A1M3),
                         uint8_t(bin2bcd(day) | A1M4 | DY_DT) };
    i2c_dev->write(buffer, 5);

    write_register(DS3231_CONTROL, ctrl | 0x01); // AI1E

    return true;
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm2(const DateTime& dt, Ds3231Alarm2Mode alarm_mode) {
    uint8_t ctrl = read_register(DS3231_CONTROL);
    if (!(ctrl & 0x04)) {
        return false;
    }

    uint8_t A2M2 = (alarm_mode & 0x01) << 7; // Minutes bit 7.
    uint8_t A2M3 = (alarm_mode & 0x02) << 6; // Hour bit 7.
    uint8_t A2M4 = (alarm_mode & 0x04) << 5; // Day/Date bit 7.
    uint8_t DY_DT = (alarm_mode & 0x08)
        << 3; // Day/Date bit 6. Date when 0, day of week when 1.
    uint8_t day = (DY_DT) ? dowToDS3231(dt.dayOfTheWeek()) : dt.day();

    uint8_t buffer[4] = { DS3231_ALARM2, uint8_t(bin2bcd(dt.minute()) | A2M2),
                         uint8_t(bin2bcd(dt.hour()) | A2M3),
                         uint8_t(bin2bcd(day) | A2M4 | DY_DT) };
    i2c_dev->write(buffer, 4);

    write_register(DS3231_CONTROL, ctrl | 0x02); // AI2E

    return true;
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm1
    @return DateTime object with the Alarm1 data set in the
            day, hour, minutes, and seconds fields
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm1() {
    uint8_t buffer[5] = { DS3231_ALARM1, 0, 0, 0, 0 };
    i2c_dev->write_then_read(buffer, 1, buffer, 5);

    uint8_t seconds = bcd2bin(buffer[0] & 0x7F);
    uint8_t minutes = bcd2bin(buffer[1] & 0x7F);
    // Fetching the hour assumes 24 hour time (never 12)
    // because this library exclusively stores the time
    // in 24 hour format. Note that the DS3231 supports
    // 12 hour storage, and sets bits to indicate the type
    // that is stored.
    uint8_t hour = bcd2bin(buffer[2] & 0x3F);

    // Determine if the alarm is set to fire based on the
    // day of the week, or an explicit date match.
    bool isDayOfWeek = (buffer[3] & 0x40) >> 6;
    uint8_t day;
    if (isDayOfWeek) {
        // Alarm set to match on day of the week
        day = bcd2bin(buffer[3] & 0x0F);
    }
    else {
        // Alarm set to match on day of the month
        day = bcd2bin(buffer[3] & 0x3F);
    }

    // On the first week of May 2000, the day-of-the-week number
    // matches the date number.
    return DateTime(2000, 5, day, hour, minutes, seconds);
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm2
    @return DateTime object with the Alarm2 data set in the
            day, hour, and minutes fields
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm2() {
    uint8_t buffer[4] = { DS3231_ALARM2, 0, 0, 0 };
    i2c_dev->write_then_read(buffer, 1, buffer, 4);

    uint8_t minutes = bcd2bin(buffer[0] & 0x7F);
    // Fetching the hour assumes 24 hour time (never 12)
    // because this library exclusively stores the time
    // in 24 hour format. Note that the DS3231 supports
    // 12 hour storage, and sets bits to indicate the type
    // that is stored.
    uint8_t hour = bcd2bin(buffer[1] & 0x3F);

    // Determine if the alarm is set to fire based on the
    // day of the week, or an explicit date match.
    bool isDayOfWeek = (buffer[2] & 0x40) >> 6;
    uint8_t day;
    if (isDayOfWeek) {
        // Alarm set to match on day of the week
        day = bcd2bin(buffer[2] & 0x0F);
    }
    else {
        // Alarm set to match on day of the month
        day = bcd2bin(buffer[2] & 0x3F);
    }

    // On the first week of May 2000, the day-of-the-week number
    // matches the date number.
    return DateTime(2000, 5, day, hour, minutes, 0);
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm1
    @return Ds3231Alarm1Mode enum value for the current Alarm1 mode
*/
/**************************************************************************/
Ds3231Alarm1Mode RTC_DS3231::getAlarm1Mode() {
    uint8_t buffer[5] = { DS3231_ALARM1, 0, 0, 0, 0 };
    i2c_dev->write_then_read(buffer, 1, buffer, 5);

    uint8_t alarm_mode = (buffer[0] & 0x80) >> 7    // A1M1 - Seconds bit
        | (buffer[1] & 0x80) >> 6  // A1M2 - Minutes bit
        | (buffer[2] & 0x80) >> 5  // A1M3 - Hour bit
        | (buffer[3] & 0x80) >> 4  // A1M4 - Day/Date bit
        | (buffer[3] & 0x40) >> 2; // DY_DT

    // Determine which mode the fetched alarm bits map to
    switch (alarm_mode) {
    case DS3231_A1_PerSecond:
    case DS3231_A1_Second:
    case DS3231_A1_Minute:
    case DS3231_A1_Hour:
    case DS3231_A1_Date:
    case DS3231_A1_Day:
        return (Ds3231Alarm1Mode)alarm_mode;
    default:
        // Default if the alarm mode cannot be read
        return DS3231_A1_Date;
    }
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm2
    @return Ds3231Alarm2Mode enum value for the current Alarm2 mode
*/
/**************************************************************************/
Ds3231Alarm2Mode RTC_DS3231::getAlarm2Mode() {
    uint8_t buffer[4] = { DS3231_ALARM2, 0, 0, 0 };
    i2c_dev->write_then_read(buffer, 1, buffer, 4);

    uint8_t alarm_mode = (buffer[0] & 0x80) >> 7    // A2M2 - Minutes bit
        | (buffer[1] & 0x80) >> 6  // A2M3 - Hour bit
        | (buffer[2] & 0x80) >> 5  // A2M4 - Day/Date bit
        | (buffer[2] & 0x40) >> 3; // DY_DT

    // Determine which mode the fetched alarm bits map to
    switch (alarm_mode) {
    case DS3231_A2_PerMinute:
    case DS3231_A2_Minute:
    case DS3231_A2_Hour:
    case DS3231_A2_Date:
    case DS3231_A2_Day:
        return (Ds3231Alarm2Mode)alarm_mode;
    default:
        // Default if the alarm mode cannot be read
        return DS3231_A2_Date;
    }
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
*/
/**************************************************************************/
void RTC_DS3231::disableAlarm(uint8_t alarm_num) {
    uint8_t ctrl = read_register(DS3231_CONTROL);
    ctrl &= ~(1 << (alarm_num - 1));
    write_register(DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void RTC_DS3231::clearAlarm(uint8_t alarm_num) {
    uint8_t status = read_register(DS3231_STATUSREG);
    status &= ~(0x1 << (alarm_num - 1));
    write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::alarmFired(uint8_t alarm_num) {
    return (read_register(DS3231_STATUSREG) >> (alarm_num - 1)) & 0x1;
}

/**************************************************************************/
/*!
    @brief  Enable 32KHz Output
    @details The 32kHz output is enabled by default. It requires an external
    pull-up resistor to function correctly
*/
/**************************************************************************/
void RTC_DS3231::enable32K(void) {
    uint8_t status = read_register(DS3231_STATUSREG);
    status |= (0x1 << 0x03);
    write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void RTC_DS3231::disable32K(void) {
    uint8_t status = read_register(DS3231_STATUSREG);
    status &= ~(0x1 << 0x03);
    write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::isEnabled32K(void) {
    return (read_register(DS3231_STATUSREG) >> 0x03) & 0x01;
}