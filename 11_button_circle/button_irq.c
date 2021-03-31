#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/irqreturn.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/wait.h>

struct gpio_key {
	int gpio;
	struct gpio_desc *gpiod;
	int flag;
	int irq;
};
static struct gpio_key *myBtn_key;
static int button_major = 0;
static struct class *button_class;

static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

#define MaxSize 128
struct QNode {
	int Data[MaxSize];
	int rear;
	int front;
};

typedef struct QNode *Queue;

int IsEmpty(Queue Q);
void AddQ(Queue PtrQ, int item);
int DeleteQ(Queue PtrQ);

int IsEmpty(Queue Q)
{
	return (Q->rear == Q->front);   //1:empty 0:not empty
}


void AddQ(Queue PtrQ, int item)
{
    if((PtrQ->rear+1)%MaxSize == PtrQ->front) {
		printk("%s,Queue full\n", __FUNCTION__);
        return;
    }
    PtrQ->rear = (PtrQ->rear+1)%MaxSize;
    PtrQ->Data[PtrQ->rear] = item;
} 


int DeleteQ(Queue PtrQ)
{
    if(PtrQ->front == PtrQ->rear) {
		printk("%s,Queue empty\n", __FUNCTION__);
        return -1;
    } else {
        PtrQ->front = (PtrQ->front+1)%MaxSize;
        return PtrQ->Data[PtrQ->front];
    }
}

static Queue irqBuff;
static ssize_t button_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	int val;

	wait_event_interruptible(gpio_key_wait, !IsEmpty(irqBuff));
	val = DeleteQ(irqBuff);
	err = copy_to_user(buf, &val, 4);
	if(err != 4) {
		return -1;
	}
	return 4;
}

static struct file_operations button_ops = {
	.owner = THIS_MODULE,
	.read  = button_read, 
};

static irqreturn_t myBtn_irq_request(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;
	val = gpiod_get_value(gpio_key->gpiod);

	printk(KERN_WARNING"key %d %d\n", gpio_key->gpio, val);
	val = (myBtn_key->gpio << 8)|val;
	AddQ(irqBuff, val);
	wake_up_interruptible(&gpio_key_wait);

	return IRQ_HANDLED;
}

static int my_button_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int count;
	enum of_gpio_flags flag;
	int i, err;

	count = of_gpio_count(node);
	if(!count) {
		printk("%s,there isn't any gpio availiable\n", __FUNCTION__);
		return -1;
	}
	
	myBtn_key = (struct gpio_key*)kzalloc(sizeof(struct gpio_key)*count, GFP_KERNEL);
	if(!myBtn_key) {
		printk("%s,kzalloc malloc failed\n", __FUNCTION__);
		return -1;
	}


	for(i=0;i<count;i++) {
		myBtn_key[i].gpio = of_get_gpio_flags(node, i, &flag);
		if(myBtn_key[i].gpio < 0) {
			printk("%s, of_get_gpio_flags failed\n", __FUNCTION__);
			return -1;
		}
		myBtn_key[i].gpiod = gpio_to_desc(myBtn_key[i].gpio);
		myBtn_key[i].flag  = flag & OF_GPIO_ACTIVE_LOW;
		myBtn_key[i].irq   = gpio_to_irq(myBtn_key[i].gpio);
		err = request_irq(myBtn_key[i].irq, myBtn_irq_request, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"myBtn_key", &myBtn_key[i]);
	}

    button_major = register_chrdev(0, "mybutton", &button_ops);
    if (button_major < 0) {
        printk(KERN_ERR "button : couldn't get a major number.\n");
        return -1;
    }

    button_class = class_create(THIS_MODULE, "button_class");
    if(IS_ERR(button_class)) {
        printk(KERN_ERR "button class: create failed\n");
		unregister_chrdev(button_major, "mybutton");
        return -1;
    }


	device_create(button_class, NULL, MKDEV(button_major, 0), NULL, "mybutton%d", 0);

	return 0;
}

static int my_button_remove(struct platform_device *pdev)
{
	struct device_node *node= pdev->dev.of_node;
	int count;
	int i;

	device_destroy(button_class, MKDEV(button_major, 0));
	class_destroy(button_class);
	unregister_chrdev(button_major, "mybutton");

	count = of_gpio_count(node);
	for(i=0;i<count;i++) {
		free_irq(myBtn_key[i].irq, &myBtn_key[i]);
	}

	kfree(myBtn_key);
	return 0;
}

static struct of_device_id mybuttons[] = {
	{ .compatible = "mybtn,btn_drv" },
	{ },
};

static struct platform_driver my_button_driver = {
	.probe  = my_button_probe,
	.remove = my_button_remove,
	.driver = {
		.name = "button_dirver",
		.of_match_table = mybuttons,
	},
};

static int gpio_button_init(void)
{
	int err;
	irqBuff = (Queue)kzalloc(sizeof(struct QNode), GFP_KERNEL);
	err = platform_driver_register(&my_button_driver);
	printk(KERN_WARNING"my button dirver init\n");
	return 0;
}

static void gpio_button_exit(void)
{
	platform_driver_unregister(&my_button_driver);
	kfree(irqBuff);
	printk(KERN_WARNING"my button dirver exit\n");
}

module_init(gpio_button_init);
module_exit(gpio_button_exit);
MODULE_LICENSE("GPL");

