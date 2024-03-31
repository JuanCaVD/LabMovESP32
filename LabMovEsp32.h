#ifndef LabMovEsp32_H
#define LabMovEsp32_H

#include "Arduino.h"
#include "Wire.h"
#include <inttypes.h>
#include "Print.h" 
#include <Adafruit_I2CDevice.h>

		/*---------------------------BME680-Parametros-----------------------------------*/
#define BME680_CONCAT_BYTES(msb, lsb)	((msb << 8) | lsb)
/** BME680 Slave Address register */
#define BME680_SLAVE_ADDRESS_DEFAULT 0x77

/** Who_am_i register */
#define BME680_CHIP_ID_REG	0xD0
/** BME680 Chip ID **/
#define BME680_CHIP_ID 			0x61

#define BME680_RESET_REG			0xE0
#define BME680_INITIATES_RESET_CMD	0xB6

#define BME680_COEFF_SIZE 41
#define BME680_CALIB1_START_REG		0x89
#define BME680_CALIB2_START_REG		0xE1

/* Calibration Parameters */
#define BME680_T2_LSB_REG	1
#define BME680_T2_MSB_REG	2
#define BME680_T3_REG			3
#define BME680_P1_LSB_REG	5
#define BME680_P1_MSB_REG	6
#define BME680_P2_LSB_REG	7
#define BME680_P2_MSB_REG	8
#define BME680_P3_REG			9
#define BME680_P4_LSB_REG	11
#define BME680_P4_MSB_REG	12
#define BME680_P5_LSB_REG	13
#define BME680_P5_MSB_REG	14
#define BME680_P7_REG			15
#define BME680_P6_REG			16
#define BME680_P8_LSB_REG	19
#define BME680_P8_MSB_REG	20
#define BME680_P9_LSB_REG	21
#define BME680_P9_MSB_REG	22
#define BME680_P10_REG		23
#define BME680_H2_MSB_REG	25
#define BME680_H2_LSB_REG	26
#define BME680_H1_LSB_REG	26
#define BME680_H1_MSB_REG	27
#define BME680_H3_REG			28
#define BME680_H4_REG			29
#define BME680_H5_REG			30
#define BME680_H6_REG			31
#define BME680_H7_REG			32
#define BME680_T1_LSB_REG	33
#define BME680_T1_MSB_REG	34
#define BME680_G2_LSB_REG	35
#define BME680_G2_MSB_REG	36
#define BME680_G1_REG			37
#define BME680_G3_REG			38

#define BME680_HUM_REG_SHIFT_VAL	4
#define	BME680_BIT_H1_DATA_MSK	  0x0F

#define BME680_RES_HEAT_RANGE_MASK	0b00110000

#define BME680_EAS_STATUS_MEASURING			0b00100000
#define BME680_EAS_STATUS_NEW_DATA			0b10000000

// BME680 Register
#define BME680_RESET_REG					0xE0
#define BME680_CONFIG_REG					0x75
#define BME680_CTRL_MEAS_REG			0x74
#define BME680_CTRL_HUM_REG				0x72
#define BME680_CTRL_GAS1_REG			0x71
#define BME680_CTRL_GAS0_REG			0x70
#define BME680_HUM_MSB_REG				0x25
#define BME680_TEMP_MSB_REG				0x22
#define BME680_PRESS_MSB_REG			0x1F
#define BME680_EAS_STATUS_REG			0x1D

// IIR Fillter Control (0x75:2-4bit)
#define BME680_IIR_FILTER_0		0b00000000
#define BME680_IIR_FILTER_1		0b00000100
#define BME680_IIR_FILTER_3		0b00001000
#define BME680_IIR_FILTER_7		0b00001100
#define BME680_IIR_FILTER_15	0b00010000
#define BME680_IIR_FILTER_31	0b00010100
#define BME680_IIR_FILTER_63	0b00011000
#define BME680_IIR_FILTER_127	0b00011100

#define BME680_CONFIG_IIR_FILTER_MASK	0b00011100

#define BME680_MODE_MASK				0b00000011

// CTRL_GAS0 Heater Off
#define BME680_CTRL_GAS0_HEATER_OFF		0b00001000
#define BME680_CTRL_GAS0_HEATER_ON		0b00000000

// CTRL_GAS1 run_gas
#define BME680_CTRL_GAS1_RUN_GAS			0b00010000
// CTRL_GAS1 run_gas MASK
#define BME680_CTRL_RUN_GAS_MASK			0b11101111

// IIR_FILTER MASK
#define BME680_CONFIG_IIR_FILTER_MASK	0b11100011
#define BME680_RES_HEAT_RANGE_MASK		0b00110000
#define BME680_RES_ERROR_MASK					0b11110000

// Gas sensor wait time multiplication factor (6-7bit)
#define BME680_GAS_WAIT_FACTOR_BIT_SHIFT		6

// Max Time (factor:64 * MaxValue:63 -> 4032)
#define BME680_GAS_WAIT_MAX_TIME		0xFC0

#define BME680_OSRS_SKIP	0b000
#define BME680_OSRS_1			0b001 //1
#define BME680_OSRS_2			0b010	//2
#define BME680_OSRS_4			0b011	//3
#define BME680_OSRS_8			0b100	//4
#define BME680_OSRS_16		0b101	//5

#define BME680_SLEEP_MODE		0b00
#define BME680_FORCED_MODE	0b01

// Over Sampling Temperature Bit point(0x74:5-7bit)
#define BME680_OSRS_TEMP_BIT_SHIFT	5
// Over Sampling Pressure Bit point (0x74:5-7bit)
#define BME680_OSRS_PRESS_BIT_SHIFT	2
// Over Sampling Humidity Bit point (0x74:0-2bit)
#define BME680_OSRS_HUM_BIT_SHIFT		0


#define BME680_COEFF_SIZE 41

#define BME680_EAS_STATUS_MEASURING			0b00100000
#define BME680_EAS_STATUS_NEW_DATA			0b10000000

// Soft Reset COMMAND
#define BME680_INITIATES_RESET_CMD	0xB6

       /*--------------------------Lcd-I2C-Parametros-----------------------------------*/

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

	/*---------------------------RTC-Parametros-----------------------------------*/

class TimeSpan;

/** Constants */
#define SECONDS_PER_DAY 86400L ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000                                              \
  946684800 ///< Unixtime for 2000-01-01 00:00:00, useful for initialization

/** DS3231 SQW pin mode settings */
enum Ds3231SqwPinMode {
	DS3231_OFF = 0x1C,            /**< Off */
	DS3231_SquareWave1Hz = 0x00,  /**<  1Hz square wave */
	DS3231_SquareWave1kHz = 0x08, /**<  1kHz square wave */
	DS3231_SquareWave4kHz = 0x10, /**<  4kHz square wave */
	DS3231_SquareWave8kHz = 0x18  /**<  8kHz square wave */
};

/** DS3231 Alarm modes for alarm 1 */
enum Ds3231Alarm1Mode {
	DS3231_A1_PerSecond = 0x0F, /**< Alarm once per second */
	DS3231_A1_Second = 0x0E,    /**< Alarm when seconds match */
	DS3231_A1_Minute = 0x0C,    /**< Alarm when minutes and seconds match */
	DS3231_A1_Hour = 0x08,      /**< Alarm when hours, minutes
									 and seconds match */
	DS3231_A1_Date = 0x00,      /**< Alarm when date (day of month), hours,
								minutes and seconds match */
	DS3231_A1_Day = 0x10        /**< Alarm when day (day of week), hours,
								minutes and seconds match */
};
/** DS3231 Alarm modes for alarm 2 */
enum Ds3231Alarm2Mode {
	DS3231_A2_PerMinute = 0x7, /**< Alarm once per minute
									(whenever seconds are 0) */
	DS3231_A2_Minute = 0x6,    /**< Alarm when minutes match */
	DS3231_A2_Hour = 0x4,      /**< Alarm when hours and minutes match */
	DS3231_A2_Date = 0x0,      /**< Alarm when date (day of month), hours
									and minutes match */
	DS3231_A2_Day = 0x8        /**< Alarm when day (day of week), hours
									and minutes match */
};

	   /*--------------------------BME680-codigo-----------------------------------*/


class BME680C
{
public:
	BME680C(int addr = BME680_SLAVE_ADDRESS_DEFAULT);
	bool begin(void);
	// set parameters
	void setParam(uint8_t osrs_temp = BME680_OSRS_8, uint8_t osrs_press = BME680_OSRS_4, uint8_t osrs_hum = BME680_OSRS_4, uint8_t filter = BME680_IIR_FILTER_3);
	bool readSensors(void);
	float SensorValor(uint8_t modos);
	void readCalibrationParameter();

private:
	void setMode(uint8_t mode);
	void writeI2c(uint8_t register_addr, uint8_t value);
	void readI2c(uint8_t register_addr, uint8_t num, uint8_t* buf);
	void readI2c(uint8_t register_addr, uint8_t num);
	uint8_t readByteI2c(uint8_t register_addr);
	int device_addr;
	uint8_t raw_data[3];
	uint8_t _temp_osrs = BME680_OSRS_16;		// Temperature OverSampling Parameter
	uint8_t _hum_osrs = BME680_OSRS_4;		// Humidity OverSampling Parameter
	uint8_t _press_osrs = BME680_OSRS_4;	// Pressure OverSampling Parameter
	uint8_t _iir_filter = BME680_IIR_FILTER_7;	// IIR Filter Parameter
	uint8_t  BME680_PRESS_PAR_10, BME680_HUM_PAR_6;
	uint16_t BME680_TEM_PAR_1, BME680_PRESS_PAR_1, BME680_HUM_PAR_1, BME680_HUM_PAR_2;
	int8_t   BME680_TEM_PAR_3, BME680_PRESS_PAR_3, BME680_PRESS_PAR_6, BME680_PRESS_PAR_7, BME680_HUM_PAR_3, BME680_HUM_PAR_4, BME680_HUM_PAR_5, BME680_HUM_PAR_7;
	int16_t  BME680_TEM_PAR_2, BME680_PRESS_PAR_2, BME680_PRESS_PAR_4, BME680_PRESS_PAR_5, BME680_PRESS_PAR_8, BME680_PRESS_PAR_9;

};




/*--------------------------Lcd-I2C-Codigo-----------------------------------*/

class LiquidCrystal_I2C : public Print {
public:
	LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
	void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
	void clear();
	void home();
	void noDisplay();
	void display();
	void noBlink();
	void blink();
	void noCursor();
	void cursor();
	void scrollDisplayLeft();
	void scrollDisplayRight();
	void printLeft();
	void printRight();
	void leftToRight();
	void rightToLeft();
	void shiftIncrement();
	void shiftDecrement();
	void noBacklight();
	void backlight();
	void autoscroll();
	void noAutoscroll();
	void createChar(uint8_t, uint8_t[]);
	void setCursor(uint8_t, uint8_t);
#if defined(ARDUINO) && ARDUINO >= 100
	virtual size_t write(uint8_t);
#else
	virtual void write(uint8_t);
#endif
	void command(uint8_t);
	void init();

	////compatibility API function aliases
	void blink_on();						// alias for blink()
	void blink_off();       					// alias for noBlink()
	void cursor_on();      	 					// alias for cursor()
	void cursor_off();      					// alias for noCursor()
	void setBacklight(uint8_t new_val);				// alias for backlight() and nobacklight()
	void load_custom_character(uint8_t char_num, uint8_t* rows);	// alias for createChar()
	void printstr(const char[]);

	////Unsupported API functions (not implemented in this library)
	uint8_t status();
	void setContrast(uint8_t new_val);
	uint8_t keypad();
	void setDelay(int, int);
	void on();
	void off();
	uint8_t init_bargraph(uint8_t graphtype);
	void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end);
	void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end);


private:
	void init_priv();
	void send(uint8_t, uint8_t);
	void write4bits(uint8_t);
	void expanderWrite(uint8_t);
	void pulseEnable(uint8_t);
	uint8_t _Addrr;
	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;
	uint8_t _numlines;
	uint8_t _cols;
	uint8_t _rows;
	uint8_t _backlightval;
};


	/*--------------------------RTC-Codigo-----------------------------------*/


class DateTime {
public:
	DateTime(uint32_t t = SECONDS_FROM_1970_TO_2000);
	DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0,
		uint8_t min = 0, uint8_t sec = 0);
	DateTime(const DateTime& copy);
	DateTime(const char* date, const char* time);
	DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
	DateTime(const char* iso8601date);
	bool isValid() const;
	char* toString(char* buffer) const;

	/*!
		@brief  Return the year.
		@return Year (range: 2000--2099).
	*/
	uint16_t year() const { return 2000U + yOff; }
	/*!
		@brief  Return the month.
		@return Month number (1--12).
	*/
	uint8_t month() const { return m; }
	/*!
		@brief  Return the day of the month.
		@return Day of the month (1--31).
	*/
	uint8_t day() const { return d; }
	/*!
		@brief  Return the hour
		@return Hour (0--23).
	*/
	uint8_t hour() const { return hh; }

	uint8_t twelveHour() const;
	/*!
		@brief  Return whether the time is PM.
		@return 0 if the time is AM, 1 if it's PM.
	*/
	uint8_t isPM() const { return hh >= 12; }
	/*!
		@brief  Return the minute.
		@return Minute (0--59).
	*/
	uint8_t minute() const { return mm; }
	/*!
		@brief  Return the second.
		@return Second (0--59).
	*/
	uint8_t second() const { return ss; }

	uint8_t dayOfTheWeek() const;

	/* 32-bit times as seconds since 2000-01-01. */
	uint32_t secondstime() const;

	/* 32-bit times as seconds since 1970-01-01. */
	uint32_t unixtime(void) const;

	/*!
		Format of the ISO 8601 timestamp generated by `timestamp()`. Each
		option corresponds to a `toString()` format as follows:
	*/
	enum timestampOpt {
		TIMESTAMP_FULL, //!< `YYYY-MM-DDThh:mm:ss`
		TIMESTAMP_TIME, //!< `hh:mm:ss`
		TIMESTAMP_DATE  //!< `YYYY-MM-DD`
	};
	String timestamp(timestampOpt opt = TIMESTAMP_FULL) const;

	DateTime operator+(const TimeSpan& span) const;
	DateTime operator-(const TimeSpan& span) const;
	TimeSpan operator-(const DateTime& right) const;
	bool operator<(const DateTime& right) const;

	/*!
		@brief  Test if one DateTime is greater (later) than another.
		@warning if one or both DateTime objects are invalid, returned value is
		  meaningless
		@see use `isValid()` method to check if DateTime object is valid
		@param right DateTime object to compare
		@return True if the left DateTime is later than the right one,
		  false otherwise
	*/
	bool operator>(const DateTime& right) const { return right < *this; }

	/*!
		@brief  Test if one DateTime is less (earlier) than or equal to another
		@warning if one or both DateTime objects are invalid, returned value is
		  meaningless
		@see use `isValid()` method to check if DateTime object is valid
		@param right DateTime object to compare
		@return True if the left DateTime is earlier than or equal to the
		  right one, false otherwise
	*/
	bool operator<=(const DateTime& right) const { return !(*this > right); }

	/*!
		@brief  Test if one DateTime is greater (later) than or equal to another
		@warning if one or both DateTime objects are invalid, returned value is
		  meaningless
		@see use `isValid()` method to check if DateTime object is valid
		@param right DateTime object to compare
		@return True if the left DateTime is later than or equal to the right
		  one, false otherwise
	*/
	bool operator>=(const DateTime& right) const { return !(*this < right); }
	bool operator==(const DateTime& right) const;

	/*!
		@brief  Test if two DateTime objects are not equal.
		@warning if one or both DateTime objects are invalid, returned value is
		  meaningless
		@see use `isValid()` method to check if DateTime object is valid
		@param right DateTime object to compare
		@return True if the two objects are not equal, false if they are
	*/
	bool operator!=(const DateTime& right) const { return !(*this == right); }

protected:
	uint8_t yOff; ///< Year offset from 2000
	uint8_t m;    ///< Month 1-12
	uint8_t d;    ///< Day 1-31
	uint8_t hh;   ///< Hours 0-23
	uint8_t mm;   ///< Minutes 0-59
	uint8_t ss;   ///< Seconds 0-59
};

/**************************************************************************/
/*!
	@brief  Timespan which can represent changes in time with seconds accuracy.
*/
/**************************************************************************/
class TimeSpan {
public:
	TimeSpan(int32_t seconds = 0);
	TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
	TimeSpan(const TimeSpan& copy);

	/*!
		@brief  Number of days in the TimeSpan
				e.g. 4
		@return int16_t days
	*/
	int16_t days() const { return _seconds / 86400L; }
	/*!
		@brief  Number of hours in the TimeSpan
				This is not the total hours, it includes the days
				e.g. 4 days, 3 hours - NOT 99 hours
		@return int8_t hours
	*/
	int8_t hours() const { return _seconds / 3600 % 24; }
	/*!
		@brief  Number of minutes in the TimeSpan
				This is not the total minutes, it includes days/hours
				e.g. 4 days, 3 hours, 27 minutes
		@return int8_t minutes
	*/
	int8_t minutes() const { return _seconds / 60 % 60; }
	/*!
		@brief  Number of seconds in the TimeSpan
				This is not the total seconds, it includes the days/hours/minutes
				e.g. 4 days, 3 hours, 27 minutes, 7 seconds
		@return int8_t seconds
	*/
	int8_t seconds() const { return _seconds % 60; }
	/*!
		@brief  Total number of seconds in the TimeSpan, e.g. 358027
		@return int32_t seconds
	*/
	int32_t totalseconds() const { return _seconds; }

	TimeSpan operator+(const TimeSpan& right) const;
	TimeSpan operator-(const TimeSpan& right) const;

protected:
	int32_t _seconds; ///< Actual TimeSpan value is stored as seconds
};

/**************************************************************************/
/*!
	@brief  A generic I2C RTC base class. DO NOT USE DIRECTLY
*/
/**************************************************************************/
class RTC_I2C {
protected:
	/*!
		@brief  Convert a binary coded decimal value to binary. RTC stores
	  time/date values as BCD.
		@param val BCD value
		@return Binary value
	*/
	static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
	/*!
		@brief  Convert a binary value to BCD format for the RTC registers
		@param val Binary value
		@return BCD value
	*/
	static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
	Adafruit_I2CDevice* i2c_dev = NULL; ///< Pointer to I2C bus interface
	uint8_t read_register(uint8_t reg);
	void write_register(uint8_t reg, uint8_t val);
};


/**************************************************************************/
/*!
	@brief  RTC based on the DS3231 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_DS3231 : RTC_I2C {
public:
	bool begin();
	void adjust(const DateTime& dt);
	bool lostPower(void);
	DateTime now();
	Ds3231SqwPinMode readSqwPinMode();
	void writeSqwPinMode(Ds3231SqwPinMode mode);
	bool setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode);
	bool setAlarm2(const DateTime& dt, Ds3231Alarm2Mode alarm_mode);
	DateTime getAlarm1();
	DateTime getAlarm2();
	Ds3231Alarm1Mode getAlarm1Mode();
	Ds3231Alarm2Mode getAlarm2Mode();
	void disableAlarm(uint8_t alarm_num);
	void clearAlarm(uint8_t alarm_num);
	bool alarmFired(uint8_t alarm_num);
	void enable32K(void);
	void disable32K(void);
	bool isEnabled32K(void);
	float getTemperature(); // in Celsius degree
	/*!
		@brief  Convert the day of the week to a representation suitable for
				storing in the DS3231: from 1 (Monday) to 7 (Sunday).
		@param  d Day of the week as represented by the library:
				from 0 (Sunday) to 6 (Saturday).
		@return the converted value
	*/
	static uint8_t dowToDS3231(uint8_t d) { return d == 0 ? 7 : d; }
};

/**************************************************************************/
/*!
	@brief  RTC using the internal millis() clock, has to be initialized before
   use. NOTE: this is immune to millis() rollover events.
*/
/**************************************************************************/
class RTC_Millis {
public:
	/*!
		@brief  Start the RTC
		@param dt DateTime object with the date/time to set
	*/
	void begin(const DateTime& dt) { adjust(dt); }
	void adjust(const DateTime& dt);
	DateTime now();

protected:
	/*!
		Unix time from the previous call to now().

		This, together with `lastMillis`, defines the alignment between
		the `millis()` timescale and the Unix timescale. Both variables
		are updated on each call to now(), which prevents rollover issues.
	*/
	uint32_t lastUnix;
	/*!
		`millis()` value corresponding `lastUnix`.

		Note that this is **not** the `millis()` value of the last call to
		now(): it's the `millis()` value corresponding to the last **full
		second** of Unix time preceding the last call to now().
	*/
	uint32_t lastMillis;
};

/**************************************************************************/
/*!
	@brief  RTC using the internal micros() clock, has to be initialized before
			use. Unlike RTC_Millis, this can be tuned in order to compensate for
			the natural drift of the system clock. Note that now() has to be
			called more frequently than the micros() rollover period, which is
			approximately 71.6 minutes.
*/
/**************************************************************************/
class RTC_Micros {
public:
	/*!
		@brief  Start the RTC
		@param dt DateTime object with the date/time to set
	*/
	void begin(const DateTime& dt) { adjust(dt); }
	void adjust(const DateTime& dt);
	void adjustDrift(int ppm);
	DateTime now();

protected:
	/*!
		Number of microseconds reported by `micros()` per "true"
		(calibrated) second.
	*/
	uint32_t microsPerSecond = 1000000;
	/*!
		Unix time from the previous call to now().

		The timing logic is identical to RTC_Millis.
	*/
	uint32_t lastUnix;
	/*!
		`micros()` value corresponding to `lastUnix`.
	*/
	uint32_t lastMicros;
};

#endif