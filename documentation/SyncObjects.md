There are two types of semaphores; image and render. They are a back and forth between the swapchain and GPU, letting the other one know when they have finished their job.
imageAvailable semaphore — signals when the swapchain has finished presenting an image and that slot is free to render into. The GPU waits on this before starting to render.
renderFinished semaphore — signals when the GPU has finished rendering. The swapchain waits on this before presenting.
So the flow is: swapchain signals imageAvailable → GPU renders → GPU signals renderFinished → swapchain presents.