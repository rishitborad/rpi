#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include "lcd_driver.h"

#define I2C_DEVICE_IDX 6
#define SLAVE_DEVICE_NAME "hi_letgo_lcd"
#define SLAVE_ADDRESS 0x27

static struct i2c_adapter *etx_i2c_adapter    = NULL;
static struct i2c_client  *etx_i2c_client_lcd = NULL; 

static const struct i2c_board_info hi_letgo_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SLAVE_ADDRESS),
};

static const struct i2c_device_id hi_letgo_lcd_id[] = {
	{ SLAVE_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, hi_letgo_lcd_id);

static int hi_letgo_lcd_write_byte(uint8_t data, uint8_t mode);
static void expanderWrite(uint8_t data);
/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_recv(etx_i2c_client_lcd, out_buf, len);
    return ret;
}

static void expanderRead(uint8_t* data, uint16_t len)
{
    int ret;
    pr_info("--> expanderRead Enter");
    ret = I2C_Read(data, len);
    if (ret < 0)
        pr_err("Error: hi_letgo_lcd_read %X", ret);
    pr_info("--> expanderRead Exit");
    return ret;
}

static void readEnable(uint8_t* data, uint16_t len)
{
    int temp = 0;
    pr_info("-> readEnable Enter");
	// expanderWrite(temp | EN_ENABLE);	// En high
	// ndelay(500);
    expanderRead(data, len);
    ndelay(500);
	// expanderWrite(temp & ~EN_ENABLE);	// En low
	// ndelay(500);		// commands need > 37us to settle
    pr_info("-> readEnable Exit: len: %d", len);
} 


/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buffer, unsigned int len)
{
    uint16_t i;
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    // pr_info("I2C_Write: %d bytes", len);
    // uint8_t *buffer;
    // buffer = kzalloc(len, GFP_KERNEL);
    // if(!buffer)
    //     return -ENOMEM;
    
    for(i=0; i<len; i++)
    {
        int ret = i2c_master_send(etx_i2c_client_lcd, &buffer[i], len);
        if(ret < 0)
        {
            dev_err(&etx_i2c_client_lcd->adapter->dev, "i2c_write failed at %d\n", i);
            return ret;
        }
    }
    return 0;
}

#define LCD_NOBACKLIGHT 0x00

static void expanderWrite(uint8_t data)
{
    int ret;
    pr_info("---> expanderWrite Enter: %X", data);
    ret = I2C_Write((unsigned char*)&data, 1);
    if (ret < 0)
        pr_err("Error: hi_letgo_lcd_write %X", ret);
    pr_info("---> expanderWrite Exit: %X", data);
}

static void pulseEnable(uint8_t data){
    pr_info("--> pulseEnable Enter: %X", data);
	expanderWrite(data | EN_ENABLE);	// En high
	ndelay(500);
    expanderWrite(data & ~EN_ENABLE);	// En low
	ndelay(500);		// commands need > 37us to settle
    pr_info("--> pulseEnable Exit: %X", data);
} 

static void write4bits(uint8_t value) {
    pr_info("-> write4bits Enter: %X", value);
	// expanderWrite(value);
	pulseEnable(value);
    pr_info("-> write4bits Exit: %X", value);
}

static int hi_letgo_lcd_write_byte(uint8_t data, uint8_t mode)
{
    int ret;
    uint8_t highnib;
    uint8_t lownib;
    uint8_t value;
    
    if (data == NULL)
        return -1;
    value = data;
    
    pr_info("hi_letgo Enter: %X", value);
    highnib = value&0xf0;
    lownib = (value<<4)&0xf0;

    write4bits(highnib|mode);
    write4bits(lownib|mode);
    pr_info("hi_letgo Exit");
    return 0;
}

static bool is_busy(void)
{
    int ret = 0;
    uint8_t read_buffer[2] = {0,0};
    readEnable(read_buffer, 2);
    pr_info("read_buffer[0]= %X",read_buffer[0]);
    pr_info("read_buffer[1]= %X",read_buffer[1]);
    if(read_buffer[0] && 0x80)
        return true;
    return false;

}

#define FOUR_BIT_MODE (0x03<<4)

static int enable_4bit_mode(void)
{
    int ret = 0;
    uint8_t value = 0;
    int decrement = 0;
    
    pr_info("Entering - Enabling 4bit mode");
    // value = FOUR_BIT_MODE | RW_WRITE | RS_IR;
    // ret |= hi_letgo_lcd_write_byte(&value);
    // mdelay(450);
    // ret |= hi_letgo_lcd_write_byte(&value);
    // mdelay(450);
    // ret |= hi_letgo_lcd_write_byte(&value);
    // mdelay(450);
    // value &= ~(0x01<<4);
    // ret |= hi_letgo_lcd_write_byte(&value);

    mdelay(16);
    expanderWrite(LCD_NOBACKLIGHT);
    
    // write4bits(0x03 << 4);
    hi_letgo_lcd_write_byte(0x03 << 4, RS_IR);
    mdelay(5); // wait min 4.1ms

    // second try
    // write4bits(0x03 << 4);
    hi_letgo_lcd_write_byte(0x03 << 4, RS_IR);
    udelay(110); // wait min 4.1ms

    // third go!
    hi_letgo_lcd_write_byte(0x03 << 4, RS_IR);
    // write4bits(0x03 << 4); 
    udelay(50);

    // finally, set to 4-bit interface
    // write4bits(0x02 << 4); 
    hi_letgo_lcd_write_byte(0x01 << 5 ,RS_IR);
    udelay(50);
    hi_letgo_lcd_write_byte(0x01 << 5 ,RS_IR);
    
    while(is_busy() && decrement)
    {
        pr_info("Busy flag set");
        mdelay(1);
        decrement--;
    }

    hi_letgo_lcd_write_byte(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS, RS_IR);

    hi_letgo_lcd_write_byte(LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON, RS_IR);

    // hi_letgo_lcd_write_byte(LCD_CLEARDISPLAY, RS_IR);
    // mdelay(2);

    // hi_letgo_lcd_write_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, RS_IR);

    // hi_letgo_lcd_write_byte(LCD_RETURNHOME, RS_IR);
    // mdelay(2);

    pr_info("Exiting - Enabling 4bit mode");
    return ret;
}

static int hi_letgo_lcd_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
    int ret = 0;
    pr_info("Probing hi_letgo_lcd");
    if(etx_i2c_client_lcd == NULL)
        pr_err("Error: hi_letgo_lcd_device is null");
    
    ret = enable_4bit_mode();
    if(ret)
        pr_err("Error: enable_4bit_mode returned error %d", ret);
    return ret;
}

static int hi_letgo_lcd_remove(struct i2c_client *client)
{
    pr_info("Removing hi_letgo_lcd");
    return 0;
}

static struct i2c_driver hi_letgo_lcd_driver = {
	.driver   = {
		.name  = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = hi_letgo_lcd_probe,
	.remove   = hi_letgo_lcd_remove,
	.id_table = hi_letgo_lcd_id,
};

static int __init hi_letgo_lcd_init(void)
{
    
    etx_i2c_adapter    = i2c_get_adapter(I2C_DEVICE_IDX);
    etx_i2c_client_lcd = i2c_new_client_device(etx_i2c_adapter, &hi_letgo_info);
    
    if (IS_ERR(etx_i2c_client_lcd)) {
        pr_err("Failed to add new i2c client");
		return -1;
	}


    pr_info("hi_letgo_lcd kernel module installed succesfully");
    return i2c_add_driver(&hi_letgo_lcd_driver);
}

static void __exit hi_letgo_lcd_exit(void)
{
	pr_info("hi_letgo_lcd kernel module removed succesfully");
    i2c_unregister_device(etx_i2c_client_lcd);
	i2c_del_driver(&hi_letgo_lcd_driver);
}

module_init(hi_letgo_lcd_init);
module_exit(hi_letgo_lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rishit Borad");
MODULE_DESCRIPTION("This is HiLetgo 2004 LCD module");
MODULE_VERSION("2:1.0");