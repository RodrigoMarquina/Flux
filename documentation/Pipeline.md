The pipeline is the bundle that is sent to the driver describing the stages our workflow will follow. The stages are: VertexInputState, InputAssemblyState, ViewportState, RasterizationState, MultisampleState, DepthStencilState, pColorBlendState.
First the VertexInputState describes the format of the vertex data coming from the vertex buffer.
Then, the InputAssemblyState tells the GPU how to interpret the vertices. What is the actual geometry?
After that we have the ViewportState which adds the viewport and the scissors which are basically the window.
Then, the RasterizationState we rasterize the triangles.
The MultisampleState is the anti-aliasing to smooth out jagged lines.
The DepthStencilState is the stage that determines the actual depth using the depth buffer.
Finally the ColorBlendState is when we discard the depth buffer and just create the pixels with the color attachment.
Finally the Layout describes the resources of the shader. Uniform Buffers, textures etc.
The pipeline is a bundle of this stages and is fully compiled and optimized for the GPU to maximize performance. If we sent stage by stage that will slow down the GPU and its bully compiled so it goes directly into the GPU. Just like how our shaders are comiled to binary, this gives us less control over it but speeds up the rendering massively.