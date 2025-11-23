// virt_input.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static struct input_dev *vdev;
static struct delayed_work vwork;
static struct workqueue_struct *v_wq;
static bool key_state = false;

/* 工作函数：切换 KEY_A 状态并上报 */
static void virt_input_work(struct work_struct *work)
{
    /* 切换按键状态 */
    key_state = !key_state;

    /* 报告事件：KEY_A 按下/松开 */
    input_report_key(vdev, KEY_A, key_state ? 1 : 0);
    input_sync(vdev);

    /* 重新调度：1s 后再次运行 */
    queue_delayed_work(v_wq, &vwork, msecs_to_jiffies(1000));
}

static int __init virt_input_init(void)
{
    int err;

    /* 分配 input_dev */
    vdev = input_allocate_device();
    if (!vdev) {
        pr_err("virt_input: failed to alloc input_dev\n");
        return -ENOMEM;
    }

    vdev->name = "vinput-demo";
    vdev->phys = "vinput/input0";
    vdev->id.bustype = BUS_VIRTUAL;

    /* 设置能力：EV_KEY + KEY_A */
    set_bit(EV_KEY, vdev->evbit);
    set_bit(KEY_A, vdev->keybit); /* KEY_A (usually code 30) */

    /* 注册设备到 input core（内核会创建 /dev/input/eventX） */
    err = input_register_device(vdev);
    if (err) {
        pr_err("virt_input: input_register_device failed: %d\n", err);
        input_free_device(vdev);
        return err;
    }

    /* 创建单线程工作队列用于周期性上报 */
    v_wq = create_singlethread_workqueue("vinput_wq");
    if (!v_wq) {
        pr_err("virt_input: create workqueue failed\n");
        input_unregister_device(vdev);
        input_free_device(vdev);
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&vwork, virt_input_work);
    queue_delayed_work(v_wq, &vwork, msecs_to_jiffies(1000));

    pr_info("virt_input: inited, device name='%s'\n", vdev->name);
    return 0;
}

static void __exit virt_input_exit(void)
{
    /* cancel work并销毁队列 */
    cancel_delayed_work_sync(&vwork);
    if (v_wq)
        destroy_workqueue(v_wq);

    /* 注销并释放 input 设备 */
    if (vdev) {
        input_unregister_device(vdev); /* 也会 free device */
        /* 注意：input_unregister_device 会在内部调用 input_free_device */
        vdev = NULL;
    }

    pr_info("virt_input: exited\n");
}

module_init(virt_input_init);
module_exit(virt_input_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Virtual input device demo (reports KEY_A every second)");

