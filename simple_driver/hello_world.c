#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>


#define SIZE_OF_MYINTARR 4

bool myBool;
char* myChar_p;
int myIntArr[SIZE_OF_MYINTARR];
bool myBool_cb;
dev_t myDev = 0;
static struct class *myClass;
static struct cdev myCdev;

static int hello_open(struct inode *inode, struct file *file);
static int hello_release(struct inode *inode, struct file *file);
static ssize_t hello_read(struct file *filep, char __user *buf, size_t len, loff_t *off);
static ssize_t hello_write(struct file *filep, const char *buf, size_t len, loff_t *off);

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write,
	.open = hello_open,
	.release = hello_release,
};


static int hello_open(struct inode *inode, struct file *file)
{
	pr_info("Driver open function called!!!\n");
	return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
	pr_info("Driver release function called!!!\n");
	return 0;
}

static ssize_t hello_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	pr_info("Driver read function called!!!\n");
	return 0;
}

static ssize_t hello_write(struct file *filep, const char *buf, size_t len, loff_t *off)
{
	pr_info("Driver write function called!!!\n");
	return len;
}

static void print_my_local_variables(void)
{
	size_t i;

	printk(KERN_INFO "myBool is %s", myBool ? "SET" : "RESET");
	printk(KERN_INFO "myBool_cb is %s", myBool_cb ? "SET" : "RESET");

	if(myChar_p)
	{
		printk(KERN_INFO "myChar_p = %s", myChar_p);
	}
	
	for(i = 0; i < SIZE_OF_MYINTARR; i++)
	{
		printk(KERN_INFO "myIntArr[%u] =%d", i, myIntArr[i]);
	}
			
}

module_param(myBool, bool, S_IWUSR|S_IRUSR);
module_param(myChar_p, charp, S_IWUSR|S_IRUSR);
module_param_array(myIntArr, int, NULL, S_IWUSR|S_IRUSR);

int notify_param(const char* val, const struct kernel_param *kp)
{
	int res = param_set_int(val, kp);
	if(res==0)
	{

		printk(KERN_INFO "myBool_cb value changed to %s", *val==1 ? "SET" : "RESET");
		print_my_local_variables();
		return 0;
	}
	return -1;
}

const struct kernel_param_ops my_param_ops = {
	.set = &notify_param,
	.get = &param_get_int,
};

module_param_cb(myBool_cb, &my_param_ops, &myBool_cb, S_IWUSR|S_IRUSR);

static int __init hello_world_init(void)
{
	printk(KERN_INFO "Initializing Hello world");
	print_my_local_variables();	
	if(alloc_chrdev_region(&myDev, 0, 1, "hello_world_dev") < 0)
	{
		pr_err("Error allocating char device major number \n");
		return -1;
	}
	printk(KERN_INFO "Major=%d, Minor=%d", MAJOR(myDev), MINOR(myDev));

	cdev_init(&myCdev, &fops);

	if(cdev_add(&myCdev, myDev,1) < 0)
	{
		pr_info("Could not add device to the system");
		goto r_class;
	}

	myClass = class_create(THIS_MODULE, "hello_class");
	if(IS_ERR(myClass))
	{
		pr_err("cannot create the struct class for the device\n");
		goto r_class;
	}

	if(IS_ERR(device_create(myClass, NULL, myDev, NULL, "hello_world_dev")))
	{
		pr_err(	"cannot create the device");
		goto r_device;
	}

	pr_info("Kernel module inserted succesfully\n");
	return 0;

	r_device:
		class_destroy(myClass);
	r_class:
		unregister_chrdev_region(myDev,1);
		return -1;
}

static void __exit hello_world_exit(void)
{
	device_destroy(myClass,myDev);
	class_destroy(myClass);	
	cdev_del(&myCdev);
	unregister_chrdev_region(myDev,1); 
	pr_info("Kernel module removed succesfully");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rishit Borad");
MODULE_DESCRIPTION("This is hello world module");
MODULE_VERSION("2:1.0");
