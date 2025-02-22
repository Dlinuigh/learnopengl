# Depth testing

在坐标系系统章节我们渲染了一个3D的容器，使用深度缓冲区来组织本应被阻挡的三角形渲染在了靠前的地方。这一章将会演示更多的关于深度值、深度缓冲区存储以及如何判断“前面”的细节。

>
In the coordinate systems chapter we've rendered a 3D container and made use of a depth buffer to prevent triangles rendering in the front while they're supposed to be behind other triangles. In this chapter we're going to elaborate a bit more on those depth values the depth buffer (or z-buffer) stores and how it actually determines if a fragment is in front.

深度缓冲区类似于颜色缓冲区（存储了线段的颜色输出的东西），它存储了与颜色缓冲区相同大小的信息（就是说一个段应该既有颜色信息也有深度信息）。深度缓冲区由窗口系统自动创建并以16、24、32位单精度浮点数存储深度值，大多数系统中是24精度的。

>
The depth-buffer is a buffer that, just like the color buffer (that stores all the fragment colors: the visual output), stores information per fragment and has the same width and height as the color buffer. The depth buffer is automatically created by the windowing system and stores its depth values as 16, 24 or 32 bit floats. In most systems you'll see a depth buffer with a precision of 24 bits.

深度测试开启时，opengl会测试深度缓冲区里面段的深度值，测试通过的彩绘进行渲染，并且深度缓冲区会更新一个深度值，否则就会放弃渲染这个段。

>
When depth testing is enabled, OpenGL tests the depth value of a fragment against the content of the depth buffer. OpenGL performs a depth test and if this test passes, the fragment is rendered and the depth buffer is updated with the new depth value. If the depth test fails, the fragment is discarded.

深度测试在段渲染开始（与下一章会讲到的stencil，模板测试）后在屏幕空间完成。屏幕空间坐标与使用`glViewport`定义的视窗相关联，可以用glsl语言里面的内建变量`gl_FragCoord`获得渲染段的坐标。这个坐标的原点是屏幕的左下角。这个变量实际上还有一个`z`，这个是段的z值，要和当前的深度缓冲区的内容比较。（数字大的更接近屏幕外侧，数字小则在内侧）

>
Depth testing is done in screen space after the fragment shader has run (and after the stencil test which we'll get to in the next chapter). The screen space coordinates relate directly to the viewport defined by OpenGL's glViewport function and can be accessed via GLSL's built-in gl_FragCoord variable in the fragment shader. The x and y components of gl_FragCoord represent the fragment's screen-space coordinates (with (0,0) being the bottom-left corner). The gl_FragCoord variable also contains a z-component which contains the depth value of the fragment. This z value is the value that is compared to the depth buffer's content.

现在大多数GPU支持硬件预深度测试，这允许深度测试在段渲染之前完成，如果测度测试不通过直接后续渲染也可以取消。

>
Today most GPUs support a hardware feature called early depth testing. Early depth testing allows the depth test to run before the fragment shader runs. Whenever it is clear a fragment isn't going to be visible (it is behind other objects) we can prematurely discard the fragment.

段渲染的代价比较高，尽可能提前判断避免渲染没有必要的，那么你应该禁止修改段的深度值，在运行时。如果在渲染的时候会修改深度值，那么预深度测试会得到错误的结果。

>
Fragment shaders are usually quite expensive so wherever we can avoid running them we should. A restriction on the fragment shader for early depth testing is that you shouldn't write to the fragment's depth value. If a fragment shader would write to its depth value, early depth testing is impossible; OpenGL won't be able to figure out the depth value beforehand.

深度测试默认是禁用的，用这个代码开启。

>
Depth testing is disabled by default so to enable depth testing we need to enable it with the GL_DEPTH_TEST option:

``` cpp
glEnable(GL_DEPTH_TEST);  
```

启用后，opengl会自动存储z值在深度缓冲区里面通过测试的那些段，其余的就丢弃了。每一帧前面都应该使用这个代码清空深度缓冲区，否则就会有上一次的结果。

>
Once enabled, OpenGL automatically stores fragments their z-values in the depth buffer if they passed the depth test and discards fragments if they failed the depth test accordingly. If you have depth testing enabled you should also clear the depth buffer before each frame using GL_DEPTH_BUFFER_BIT; otherwise you're stuck with the depth values from last frame:

``` cpp
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
```

未来可能有一些你只是执行深度测试但是并不更新深度缓冲区的场景，一般来说你可以暂时用一个只读的深度缓冲区来实现。将深度掩码设置为0就可以禁用写入深度缓冲区。

>
There are certain scenarios imaginable where you want to perform the depth test on all fragments and discard them accordingly, but not update the depth buffer. Basically, you're (temporarily) using a read-only depth buffer. OpenGL allows us to disable writing to the depth buffer by setting its depth mask to GL_FALSE:

``` cpp
glDepthMask(GL_FALSE);  
```

注意只有在开启深度测试才有效。

>
Note that this only has effect if depth testing is enabled.

## Depth test function

opengl允许修改深度测试比较操作函数，这将可以控制深度测试的通过条件。用下面的函数控制比较操作符。

>
OpenGL allows us to modify the comparison operators it uses for the depth test. This allows us to control when OpenGL should pass or discard fragments and when to update the depth buffer. We can set the comparison operator (or depth function) by calling glDepthFunc:

``` cpp
glDepthFunc(GL_LESS);  
```

下面的函数接受下面表格里面列出的函数。

>
The function accepts several comparison operators that are listed in the table below:

|Function | Description|
|---|---|
|GL_ALWAYS | The depth test always passes.|
|GL_NEVER  |The depth test never passes.|
|GL_LESS  |Passes if the fragment's depth value is less than the stored depth value.|
|GL_EQUAL | Passes if the fragment's depth value is equal to the stored depth value.|
|GL_LEQUAL | Passes if the fragment's depth value is less than or equal to the stored depth value.|
|GL_GREATER | Passes if the fragment's depth value is greater than the stored depth value.|
|GL_NOTEQUAL | Passes if the fragment's depth value is not equal to the stored depth value.|
|GL_GEQUAL  |Passes if the fragment's depth value is greater than or equal to the stored depth value.|

默认的函数是`GL_LESS`，更小的通过。
>
By default the depth function GL_LESS is used that discards all the fragments that have a depth value higher than or equal to the current depth buffer's value.

展示一下效果，我们设置了一个简单的场景包含两个带纹理的方块位于一个带纹理的地板上，没有灯光。这里是源代码。
>
Let's show the effect that changing the depth function has on the visual output. We'll use a fresh code setup that displays a basic scene with two textured cubes sitting on a textured floor with no lighting. You can find the source code here.

代码里面我们设置了深度函数为总是通过。
>
Within the source code we changed the depth function to GL_ALWAYS:

``` cpp
glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_ALWAYS);
```

这模拟了不开启深度测试的情况，所有的段都能被看到即使不在前面，因为我们最后绘制了地板，所以地板会覆盖之前容器里面的已经写入的段。
>
This simulates the same behavior we'd get if we didn't enable depth testing. The depth test always passes so the fragments that are drawn last are rendered in front of the fragments that were drawn before, even though they should've been at the front. Since we've drawn the floor plane last, the plane's fragments overwrite each of the container's previously written fragments:

![Depth testing in OpenGL with GL_ALWAYS as depth function](depth_testing_func_always.png)
设置为更小的通过，就是我们以前用过的，
>
Setting it all back to GL_LESS gives us the type of scene we're used to:

![Depth testing in OpenGL with GL_LESS as depth function](depth_testing_func_less.png)
这里我来插播解释：地板的上面因为是最后渲染的所以把方块的下半部分遮挡了，而位置关系上他应该是在后面被遮挡的那个。设置为更小的，首先你要知道深度是根据相机的位置判断的，不是里面的那个坐标系z更小就行，而是相对于相机更远的也就被遮挡，z更大。

## Depth value precision

深度缓冲区，包含了0-1的深度值，比较这些通过视窗看到的场景里面的物体的z值，这些z值可以是投影的两个平面之间的任何值，我们需要将这些空间里的z值通过转换函数调整到`[0,1]`，一个可行的方案是线性转换，下面的线性方程将z值转换成深度值。
>
The depth buffer contains depth values between 0.0 and 1.0 and it compares its content with the z-values of all the objects in the scene as seen from the viewer. These z-values in view space can be any value between the projection-frustum's near and far plane. We thus need some way to transform these view-space z-values to the range of [0,1] and one way is to linearly transform them. The following (linear) equation transforms the z-value to a depth value between 0.0 and 1.0:

$$Fdepth=\frac{z−near}{far−near}$$
上面说的`near`和`far`是可视区域的靠近相机的平面与远离相机的平面.(补课:我承认之前从没仔细看过内容.是这样的这个词`frustum`是一个相机的四棱台,因为相机的画幅从近到远变大,所以就是四棱台,而非圆台是因为目前的相机是矩形的.这个四棱台小的矩形是靠近相机的,大的矩形是原理的,这个呈现距离完全可控,类似于望远镜调整.)上面只需要一个输入z就行了,(补课:这个z是之前指向屏幕外侧的,外侧就是屏幕显示的方向,该方向为正方向.x与y仍旧是屏幕坐标.由于相机一般是指向内侧的,可能会存在偏角,但是不妨碍我们使用这两个平面与z轴进行投影得到最终z轴.)
>
Here _near_ and _far_ are the near and far values we used to provide to the projection matrix to set the visible frustum (see coordinate Systems). The equation takes a depth value z

上面的转换将使得z值为下方的呈现.
>
within the frustum and transforms it to the range [0,1]. The relation between the z-value and its corresponding depth value is presented in the following graph:

![Graph of depth values in OpenGL as a linear function](linear.png)
注意，靠近两侧平面的接近0和1.
>
Note that all equations give a depth value close to 0.0 when the object is close by and a depth value close to 1.0 when the object is close to the far plane.
但是经验上，线性的深度缓冲区几乎不用，因为投影的规律：近大远小，这是与反函数相关联的，那么通过反函数在近处可以得到更加精确的深度。
>
In practice however, a linear depth buffer like this is almost never used. Because of projection properties a non-linear depth equation is used that is proportional to 1/z. The result is that we get enormous precision when z is small and much less precision when z is far away.
因为非线性函数与z的反函数相关，而z值[1.0,2.0]将会得到[0.5,1.0]之间的深度值，也就是50%，而后面[50.0,100.0]的z值只占2%，这样的方程再引入两个平面。也就是下面的公式：
>
Since the non-linear function is proportional to 1/z, z-values between 1.0 and 2.0 would result in depth values between 1.0 and 0.5 which is half of the [0,1] range, giving us enormous precision at small z-values. Z-values between 50.0 and 100.0 would account for only 2% of the [0,1] range. Such an equation, that also takes near and far distances into account, is given below:

$$Fdepth=\frac{1/z-1/near}{1/far-1/near}$$

不用担心你看不懂这个公式，重点是记住深度缓冲区里面的深度值不是线性映射z值就行了，可以在下图看到z值与深度值的对应关系。
>
Don't worry if you don't know exactly what is going on with this equation. The important thing to remember is that the values in the depth buffer are not linear in clip-space (they are linear in view-space before the projection matrix is applied). A value of 0.5 in the depth buffer does not mean the pixel's z-value is halfway in the frustum; the z-value of the vertex is actually quite close to the near plane! You can see the non-linear relation between the z-value and the resulting depth buffer's value in the following graph:

![Graph of depth values in OpenGL as a non-linear function](non_linear.png)
如你所见，深度值几乎被小的z值所左右，近的物体的深度值很精准，这个公式直接运用在投影矩阵上。所以每次进行物体坐标系、视图、屏幕之间的坐标转换都会自动使用。
>
As you can see, the depth values are greatly determined by the small z-values giving us large depth precision to the objects close by. The equation to transform z-values (from the viewer's perspective) is embedded within the projection matrix so when we transform vertex coordinates from view to clip, and then to screen-space the non-linear equation is applied.

如果我们将深度值可视化那么就会更加清晰。
>
The effect of this non-linear equation quickly becomes apparent when we try to visualize the depth buffer.

## Visualizing the depth buffer

我们知道`gl_FragCoord`变量里面的`z`就是深度值，如果我们作为颜色输出这个值我们可以显示场景中所有段的深度值。
>
We know that the z-value of the built-in gl_FragCoord vector in the fragment shader contains the depth value of that particular fragment. If we were to output this depth value of the fragment as a color we could display the depth values of all the fragments in the scene:

``` glsl
void main()
{
    FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}  
```
如果你运行了代码，你可能会注意到都是白色的，使得似乎所有的深度值都是最大值1,所以没有接近0的值而更黑的段呢？
>
If you'd then run the program you'll probably notice that everything is white, making it look like all of our depth values are the maximum depth value of 1.0. So why aren't any of the depth values closer to 0.0 and thus darker?
在前面的章节里我们描述了深度值在屏幕空间下的是非线性的，对于小的值具有更高的精度，而对于大的z值有着更小的精度，深度值以一个相当快的速度接近1,如果我们小心地接近一个物体，你最终会注意到颜色变黑，它们的z值变小。如下图：
>
In the previous section we described that depth values in screen space are non-linear e.g. they have a very high precision for small z-values and a low precision for large z-values. The depth value of the fragment increases rapidly over distance so almost all the vertices have values close to 1.0. If we were to carefully move really close to an object you may eventually see the colors getting darker, their z-values becoming smaller:

![Depth buffer visualized in OpenGL and GLSL](depth_testing_visible_depth.png)

这清晰地表明了深度值的非线性性，近的位置的物体在深度值上影响更大，只移动几英尺会导致颜色从黑色到完全白色。

>
This clearly shows the non-linearity of the depth value. Objects close by have a much larger effect on the depth value than objects far away. Only moving a few inches can result in the colors going from dark to completely white.

但是我们可以将非线性的深度值转换成线性的，只需要执行反过程并加上一个线性处理就行。这样可以得到一个线性的深度值。
>
We can however, transform the non-linear depth values of the fragment back to its linear sibling. To achieve this we basically need to reverse the process of projection for the depth values alone. This means we have to first re-transform the depth values from the range [0,1] to normalized device coordinates in the range [-1,1]. Then we want to reverse the non-linear equation (equation 2) as done in the projection matrix and apply this inversed equation to the resulting depth value. The result is then a linear depth value.

首先我们轻松转换深度值为NDC。
>
First we transform the depth value to NDC which is not too difficult:

``` cpp
float ndc = depth * 2.0 - 1.0;
```
然后使用得到的NDC值代入线性深度求解函数中。
>
We then take the resulting ndc value and apply the inverse transformation to retrieve its linear depth value:

``` cpp
float linearDepth = (2.0 _near_ far) / (far + near - ndc * (far - near));
```

这个方程源于从投影矩阵中计算非线性的深度值。对于感兴趣的读者，数学推导过程见[这篇数学味十足的文章](http://www.songho.ca/opengl/gl_projectionmatrix.html)，这也解释了方程的来源。
>
This equation is derived from the projection matrix for non-linearizing the depth values, returning depth values between near and far. This [math-heavy article](http://www.songho.ca/opengl/gl_projectionmatrix.html) explains the projection matrix in enormous detail for the interested reader; it also shows where the equations come from.

下面的完整的glsl代码
>
The complete fragment shader that transforms the non-linear depth in screen-space to a linear depth value is then as follows:

``` glsl
# version 330 core

out vec4 FragColor;

float near = 0.1;
float far  = 100.0;
  
float LinearizeDepth(float depth)
{
    float z = depth _2.0 - 1.0; // back to NDC
    return (2.0_ near _far) / (far + near - z_ (far - near));
}

void main()
{
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    FragColor = vec4(vec3(depth), 1.0);
}
```
因为两个平面之间的线性深度值将会在1以上而显示为白色。通过`main`函数里面，除`far`的做法将线性的深度值转换到了[0,1]区间内。这样我们可以看到随着相机靠近物体，场景逐渐更亮，显示上更好了。
>
Because the linearized depth values range from near to far most of its values will be above 1.0 and displayed as completely white. By dividing the linear depth value by `far` in the `main` function we convert the linear depth value to the range [0, 1]. This way we can gradually see the scene become brighter the closer the fragments are to the projection frustum's `far` plane, which works better for visualization purposes.

现在运行程序能得到与距离线性关系的深度值。尝试移动场景来看线性方式变化的深度值。
>
If we'd now run the application we get depth values that are linear over distance. Try moving around the scene to see the depth values change in a linear fashion.

![Depth buffer visualized in OpenGL and GLSL as linear values](depth_testing_visible_linear.png)

基本上都是黑色，因为这些物体还算靠近相机的，最远的深度值目前定在了100，近的是0.1。
>
The colors are mostly black because the depth values range linearly from the near plane (0.1) to the far plane (100) which is still quite far away from us. The result is that we're relatively close to the `near` plane and therefore get lower (darker) depth values.

## Z-fighting

两个相近的物体计算上因为非线性导致深度值无法区分前后。结果就是一直在切换顺序，闪烁。这叫做z冲突，因为看上去两个在争谁在前面。
>
A common visual artifact may occur when two planes or triangles are so closely aligned to each other that the depth buffer does not have enough precision to figure out which one of the two shapes is in front of the other. The result is that the two shapes continually seem to switch order which causes weird glitchy patterns. This is called z-fighting, because it looks like the shapes are fighting over who gets on top.

很少的地方能够注意到z冲突。举一个之前没注意到的例子，盒子的底部与地板的上表面由于很接近但是有明确的前后之分会导致没办法确定谁在前。
>
In the scene we've been using so far there are a few spots where z-fighting can be noticed. The containers were placed at the exact height of the floor which means the bottom plane of the container is coplanar with the floor plane. The depth values of both planes are then the same so the resulting depth test has no way of figuring out which is the right one.

如果你把相机移动到盒子的内部就能清晰地看到，底部一直在盒子和地板之间切换。

>
If you move the camera inside one of the containers the effects are clearly visible, the bottom part of the container is constantly switching between the container's plane and the floor's plane in a zigzag pattern:

![Demonstration of Z-fighting in OpenGL](depth_testing_z_fighting.png)
z冲突是一个常见的深度缓冲区问题，经常能够注意到当物体很远的时候，因为更大的z轴的精度很差。并不能被完全组织，但是有一些小技巧可以缓和或者消弭z冲突在你场景中的影响。

>
Z-fighting is a common problem with depth buffers and it's generally more noticeable when objects are further away (because the depth buffer has less precision at larger z-values). Z-fighting can't be completely prevented, but there are a few tricks that will help to mitigate or completely prevent z-fighting in your scene.

## Prevent z-fighting

首要的技巧是，不要让物体之间台接近导致一些三角形几乎重合，通过加入一点小偏移可以完全移除之间的z冲突。在盒子与平面的场景下，可以轻松将盒子移到平面的上方一点点（不会重合的那个方向）。小位移不容易被注意但是可以完全消除z冲突影响。但是这要求人工的干预每一个物体并测试以确保完全不会出现z冲突。
>
The first and most important trick is never place objects too close to each other in a way that some of their triangles closely overlap. By creating a small offset between two objects you can completely remove z-fighting between the two objects. In the case of the containers and the plane we could've easily moved the containers slightly upwards in the positive y direction. The small change of the container's positions would probably not be noticeable at all and would completely reduce the z-fighting. However, this requires manual intervention of each of the objects and thorough testing to make sure no objects in a scene produce z-fighting.

第二个技巧是将相机的最近的平面挪远一点，上一章里面我们讨论过，靠近最近平面的精度非常大，所以如果我们将最近的平面挪远一点，我们可以让一整个区域都有高精度。但是设置最近的平面太远会导致近处的物体裁减，所以又引出了如何调整实验来得到最佳近距平面的问题。
>
A second trick is to set the near plane as far as possible. In one of the previous sections we've discussed that precision is extremely large when close to the near plane so if we move the near plane away from the viewer, we'll have significantly greater precision over the entire frustum range. However, setting the near plane too far could cause clipping of near objects so it is usually a matter of tweaking and experimentation to figure out the best near distance for your scene.

另一个非常不错的技巧是使用更高精度的深度缓冲区，代价是一点点性能损失。大多数深度缓冲区是24位的，但是现在大多数GPU支持32位的深度缓冲区，增加一点点精度是可行的。所以花费一点点性能就能得到更精确的深度测试来减少z冲突的影响。
>
Another great trick at the cost of some performance is to use a higher precision depth buffer. Most depth buffers have a precision of 24 bits, but most GPUs nowadays support 32 bit depth buffers, increasing the precision by a significant amount. So at the cost of some performance you'll get much more precision with depth testing, reducing z-fighting.

上面讨论的三种方法是最常用而且容易实现的消除z冲突的方法。还有一些麻烦的但是也不能完全消除z冲突。z冲突是一个普遍问题，但是如果你正确地结合上面的方法使用将不会需要处理那么多冲突情况。
>
The 3 techniques we've discussed are the most common and easy-to-implement anti z-fighting techniques. There are some other techniques out there that require a lot more work and still won't completely disable z-fighting. Z-fighting is a common issue, but if you use the proper combination of the listed techniques you probably won't need to deal with z-fighting that much.
