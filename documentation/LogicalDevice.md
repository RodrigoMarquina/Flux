A logical device is a contract between your application and the physical device. It specifies the requirements of your application. We select the specific queue family and the amount of queues we need for the application and enable extensions like the swapchain. With this requirements, the driver can optimize and customize the physical device to the needs of your application. 
Queuefamilies are a group of queues with specific specializations. For example:
Queue family 0: supports graphics + compute → has 4 queues
Queue family 1: supports transfer only → has 2 queues
Another important responsibility of logical devices is that they own the reference to the queue, this is called a queue handle. The driver's scheduler manages your submissions to the queue with this.