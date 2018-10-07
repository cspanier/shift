# Vulkan
Vulkan being a low level API requires the application to handle everything from memory allocation, through object lifetime tracking, to synchronization without much help of the driver. Thus, these requirements need to be carefully translated and put together into a renderer that now acts like a driver itself.

## Key concepts

* **Parallel allocation and uploading of long living resources like textures or mesh data**  
The engine needs to feed the GPU with data using separate command_queues for uploading data through DMA without blocking draw/compute calls on the primary command_queues. It is not the renderer's responsibility to load the actual data from disk or generating it using some CPU based algorithm. However, the renderer must provide an interface to allocate and map memory to write the data to, to avoid an unnecessary memory copy operation.

* **Dynamic allocation of short living resources like command buffers on a per-task basis**  
It is important to scale the renderer to as many threads as there are physical cores on the host system to exploit the maximum hardware parallelism available. However, many functions require short living objects allocated dynamically from a pool. The more threads share a given pool the higher the chance that calls block on an access mutex. There must be a way to allocate objects on an per-worker task object pool to avoid blocking calls.

*  **Object lifetime tacking**  
In Vulkan it is particularly difficult to track an object's lifetime because even though the API requires one to state any dependencies during object construction, it does not support any query functions later on. As a consequence we must track dependencies and must not delete any object that is being used by another one still alive. Actually it is even worse, because objects might still be in use by the GPU or they might be (indirectly) referenced by some command on a command queue.

* **Handling of swapchain dependencies**  
A swapchain contains multiple images, which are being used by an image_view per image, which are being used by a framebuffer per image_view, which are referenced during command buffer begin needed every frame.  Each framebuffer uses a separate descriptor_set usually containing the same information. The dependency on the swapchain is passed through many components, which doesn't quite help reducing overall complexity.

## Synchronization

In Vulkan, there are four forms of concurrency during execution:

* **between the host and device**  
Fences can be used by the host to determine completion of execution of queue operations.  
  * [vkCreateFence](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCreateFence)
  * [vkDestroyFence](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkDestroyFence)
  * [vkGetFenceStatus](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkGetFenceStatus)
  * [vkResetFences](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkResetFences)
  * [vkWaitForFences](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkWaitForFences)
    
* **between the queues**  
Semaphores are used to coordinate queue operations both within a queue and between different queues.
  * [vkCreateSemaphore](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCreateSemaphore)
  * [vkDestroySemaphore](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkDestroySemaphore)
  * [vkQueueSubmit](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkQueueSubmit)

* **between queue submissions**  
Events represent a fine-grained synchronization primitive that can be used to gauge progress through a sequence of commands executed on a queue by Vulkan.
  * [vkCreateEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCreateEvent)
  * [vkDestroyEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkDestroyEvent)
  * [vkGetEventStatus](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkGetEventStatus)
  * [vkSetEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkSetEvent)
  * [vkResetEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkResetEvent)
  * [vkCmdSetEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdSetEvent)
  * [vkCmdResetEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdResetEvent)
  * [vkCmdWaitEvents](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdWaitEvents)

* **between commands within a command buffer**  
More specifically:
  * Between commands before a [vkCmdSetEvent](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdSetEvent) command and commands after a [vkCmdWaitEvents](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdWaitEvents) command in the same queue, using the same event.
  * Between commands in a lower numbered subpass (or before a render pass instance) and commands in a higher numbered subpass (or after a render pass instance), where there is a subpass dependency between the two subpasses (or between a subpass and VK_SUBPASS_EXTERNAL).  
[vkCreateRenderPass](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCreateRenderPass)
  * Between commands before a pipeline barrier and commands after that pipeline barrier in the same queue (possibly limited to within the same subpass).  
[vkCmdPipelineBarrier](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#vkCmdPipelineBarrier)

## Data exchange between the host and device

There are a number of ways how to transfer information between the host and device. Buffer and image memory may be initialized from the host by one of the following techniques:
* Bulk data like textures, vertex buffers, or index buffers should be transferred using device only memory and temporary staging buffers/images in shared memory. This is also the preferred way to upload data that is seldom written and never read by the host. Memory used for staging buffers/images may be recycled once the upload is complete. Note that copy operations should be performed on a separate command queue that can process DMA operations in parallel to draw/compute command queues and that these operations need to be synchronized using semaphores.
* Small amounts of data may be transferred in both directions using  coherent memory accessible to both host and device. This memory allows the host to directly map and write into/read from device memory. The amount of this type of memory is rather limited and also slower than device only memory.
* Using non-coherent memory accessible to both host and device is very comparable to coherent shared memory, but may be a little bit faster. However, access to this type of memory from both host and device needs to be synchronized (e.g. using events).
* Very small amounts of data that need to be transferred from host to device can be injected directly into command buffers (ToDo: Link to command used for this). This technique is mostly useful for dynamically computed shader parameters that are not reused very often by different draw calls.

## Object lifetime

* We don't want to track each object pushed to some command buffer until that command buffer has fully processed on the GPU. Instead, we simply assume that command buffers are processed after at most n frames. This allows us to significantly reduce the number of atomic operations and dynamic list management storing references to such objects.

## Presentation
Fundamentally there are three timelines.

* CPU timeline: commands get queued here
* GPU timeline: queues execute here
* Presentation Engine (PE) timeline: outside of Vulkan

Section 6.6 of the core Vulkan spec explains how semaphores can be used to create dependency chains on the GPU timeline, and how fences can be used to synchronize the GPU and CPU timelines.

The PE timeline is separate from Vulkan. AcquireNextImage and QueuePresent provide a mechanism to synchronize the CPU and GPU timelines with the PE timeline.

* AcquireNextImage's semaphore can be used in a dependency chain to express a dependency on acquiring an image from the PE.
* AcquireNextImage's fence can be used to synchronize CPU operations with acquiring an image from the PE.
* A wait semaphore passed into QueuePresent makes the PE wait on a Vulkan dependency chain.

The timeout parameter to AcquireNextImage serves none of these functions.

AcquireNextImage interfaces with the PE in the following ways:

* Gets from the PE the index of an acquired image
* Arranges for the PE to signal a Vulkan semaphore and/or a Vulkan fence when it is safe for the Vulkan application to operate on the image.

AcquireNextImage is supposed to return the image index as soon as it has accomplished these things. The timeout parameter to AcquireNextImage bounds how long AcquireNextImage can wait to acquire an image from the PE.

If fence and semaphore are not already signaled by the end of AcquireNextImage then it is the PE that asynchronously signals the semaphore and fence. Stated another way, the semaphore passed into AcquireNextImage can extend a Vulkan dependency chain such that the dependency chain waits for the PE to signal semaphore. It is not AcquireNextImage's responsibility to block until the PE has signaled.

E.g., on some platforms it may be illegal for the app to attempt to queue a wait operation for the VkSemaphore prior to the PE having queued a signal operation for the platform's underlying sync primitive. On such a platform AcquireNextImage would wait (with timeout) until the PE has queued a semaphore signal operation. But AcquireNextImage would not wait for the semaphore to transition to the signaled state--the app does that through its execution dependency chain. The PE might not be able to queue the signal operation until e.g. the next vblank event.

## Texture streaming

Textures do have multiple mip-map levels. While loading a scene it is preferable to display a scene using lower resolution textures rather than skipping objects that have not yet fully loaded or even displaying no image at all (loading screen). Loading single mip-map levels to device memory is pretty much straight forward. Fences may be used to tell the host that an upload is complete. However, there are currently three known ways of how to preventing a shader from accessing not yet uploaded parts of an image:  
* Creating separate image objects for each range of possible mip-map levels. Each image requires a separate image_view because these are statically bound to an image object. Memory for these images should be shared by overlapping, which reduces performance. Thus, this approach is not feasable.
* Creating a single image but multiple image_view objects, one for each possible mip-map level range and all bound to this same image. This is probably the most reasonable solution.
* Creating a single image and single image_view, but one sampler objects per possible mip-map level range. There can be different samplers per texture(view), which would require to blow up these by a factor of n.

(ToDo: Is the sampler's minLod value added to the image_view's minLod value to compute the absolute minLod value?)

Usually image and buffer resources in Vulkan must be bound completely and contiguously to a single device memory object. This binding must be done before the resource is used, and the binding is immutable for the lifetime of the resource. Thus, all partially loaded textures would already require the full amount of memory on the device, even though most of the memory would not yet be initialized.  
Devices that support sparse resources relax these restrictions. Each texture mip-map level may be allocated and bound dynamically to a sparse texture object. The downside is that additional logic is needed to manage synchronization between changing the image's memory mapping and those using the image.
Binding sparse resource memory is a queue operation rather than a host operation and needs a special type of queue. Note that usually the graphics queue supports sparse memory binding but the transfer queue does not.

ToDo: Explain [mip tail regions](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#sparsememory-miptail) and their implication with texture streaming.

## Render Pipeline

Designing a flexible render pipeline is a rather complex task. Vulkan requires exact description of every state and all bindings for each shader pipeline object. A render pipeline's task is to define common state and bindings for a series of shader pipelines and reuse as many descriptor sets when switching the shader pipeline object as possible. A typical Vulkan render loop looks like this:

```
for each view {
  for each shader {
    bind shader pipeline
    if first shader in loop
      bind view resources      // camera, environment...
    bind shader resources      // shader control values
    for each material {
      bind material resources  // material parameters and textures
      for each object {
        bind object resources  // object transforms
        draw object
      }
    }
  }
}
```

## Multiview Rendering
Multiview - also known as Simultaneous Multi Projection (SMP) - is a technique to automatically duplicate all draw calls to a set of different render targets, views and attachments.  
Support for multiview rendering can be enabled by chaining a [`VkPhysicalDeviceMultiviewFeatures`](https://www.khronos.org/registry/vulkan/specs/1.1-khr-extensions/html/vkspec.html#VkPhysicalDeviceMultiviewFeatures) instance to the `VkDeviceCreateInfo::pEnabledFeatures` field.  
Multiview is used by chaining a [`VkRenderPassMultiviewCreateInfo`](https://www.khronos.org/registry/vulkan/specs/1.1-khr-extensions/html/vkspec.html#VkRenderPassMultiviewCreateInfo) instance to the `VkRenderPassCreateInfo::pNext` member.
Also see [GLSL Spec](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_multiview.txt).

## Random thoughts

* **Level of Detail**  
The renderer may skip an object or replace the object with a lower detail one if
  * the object's resources are not yet fully loaded into device accessible memory
  * the object's details exceed what can be presented on screen because the object is scaled too small/is too far away
  * the renderer's frames per second target cannot be met (e.g. due to too many objects on screen)

The renderer will first compute a LoD level for each object depending on its size on screen. The renderer is then allowed to further reduce the LoD by a configurable number of levels depending on whether resources are missing or due to performance reasons. It may only skip the whole object if it is not flagged as essential, or otherwise the renderer must block and wait for the lowest allowed LoD level's resources to be available.

## Memory Barriers

| Access flag                                  | Supported pipeline stages |
| -------------------------------------------- | ------------------------- |
| VK_ACCESS_INDIRECT_COMMAND_READ_BIT          | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
| VK_ACCESS_INDEX_READ_BIT                     | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
| VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT          | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
| VK_ACCESS_UNIFORM_READ_BIT                   | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHAD ER_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT           |
| VK_ACCESS_INPUT_ATTACHMENT_READ_BIT          | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
| VK_ACCESS_SHADER_READ_BIT                    | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT           |
| VK_ACCESS_SHADER_WRITE_BIT                   | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT           |
| VK_ACCESS_COLOR_ATTACHMENT_READ_BIT          | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT         | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
| VK_ACCESS_TRANSFER_READ_BIT                  | VK_PIPELINE_STAGE_TRANSFER_BIT |
| VK_ACCESS_TRANSFER_WRITE_BIT                 | VK_PIPELINE_STAGE_TRANSFER_BIT |
| VK_ACCESS_HOST_READ_BIT                      | VK_PIPELINE_STAGE_HOST_BIT |
| VK_ACCESS_HOST_WRITE_BIT                     | VK_PIPELINE_STAGE_HOST_BIT |
| VK_ACCESS_MEMORY_READ_BIT                    | N/A |
| VK_ACCESS_MEMORY_WRITE_BIT                   | N/A |
