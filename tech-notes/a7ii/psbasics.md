# PS Basics

Many ways to achieve the same thing.

### Workspace

* Document window: photo workspace
* Left: tool palette
  * color well (bottom). foreground color (painting / text / fill color; defaults to black), background color (canvas area color; defaults to white)
* Right: palette options (docked). Choose which palettes to display in Window, or reset. Each may have grouped tabs.

### Layers

* Stacked sheets of (translucent) paper each containing parts of your image. Layers on top obscure the ones below if opaque. Fundamental to photoshop.
* Toggling layer on/off: affects visuals on screen as well as when exporting images.
* Never work on the background layer directly. Especially if you've changes stacked on changes and suddenly finding yourself wanting some original pixels.
* You can do retroactive editing with layers, e.g. re-edit an existing curves layer.
* Layers offer flexibility in how it's blended in.
  * opacity
  * blend modes
* Layer mask: which areas of your image are affected by changes happening in that layer. (white-and-black thumbnail by your layer where black suggests not to apply the changes of this layer here).

### Layers in detail

* Adjusting exposure with **levels**
  * Level lets you adjust the histogram, and gives separate control over shadows, midtones, highlights
  * We can do it via image->adjustment->levels, but don't do it this way since this would be editting the background copy. Instead, do it with a levels adjustment layer
  * Normally, we'd like our image to take advantage of the full spectrum of the histogram. If not, our image may appear washed-out / low contrast.
    * To correct this, we can drag the black point and white point sliders so that they each touch the first black and white pixels of the image. This remaps the tonal distribution of the image. (standard adjustment).
    * The midpoint slider controls where the midpoint grey of the image is mapped to. Dragging it right darkens the image as the midpoint grey is mapped to a brighter pixel. As you drag it right you may lose contrast between midpoint and your highlights, to recover some contrast with highlight areas, consider white point further left. This means losing details: more pixels are turned pure white. To know how much you are clipping, hold on **opt** while dragging a black / white slider.
(Does lightroom basic adjustment works in a similar fashion as remapping black / white / shadow / hightlight pixels in PS levels?)
* Improving contrast with **curves**
  * Adjust tonal values (distribution of light and dark) of your image.
  * Also works with the histogram but more sophisticated than levels (curves is a superset of what levels do).
  * "Curve". Y-axis: output, destination image. X-axis: input, source image. We start off with a diagonal line, suggesting `f(x) = x`. We can then adjust the mapping by placing fixed points and let PS do the extrapolation.
  * The standard adjustment to fill the entire histogram then becomes mapping `[0, x_1]` to 0, `[x_2, 255]` to 255, and linear between `[x_1, x_2]` such that the contrast is magnified. Curves panel also has this same black / white points slider.
  * Similarly, to bring up contrast we usually do S-shaped curve: up the highlights and dim the shadows, such the mapping line in midtone is steeper. Note that this is done at the expense of making the mapping in blacks and whites areas less steep. Think of this as "contrast budget", and all curves allow you to do is to spend it in some areas of the histogram at the cost of sacrificing other areas in the histogram.
  * Knowing this, say your histogram has two peaks one in highlight and one in shadows, you can increase the contrast of those peaks by making local S-curves.
  * Some color shift may happen with this adjustment (aberation). If this is undesirable, try "blend mode - luminosity" (only affect the brightness of the pixels with this layer, don't change their color)
  * Drag points off to get rid of them.
  * Curves eye-dropper allows you to pick spots you define as black (RGB 20, 20, 20) / white (240, 240, 240)  / midpoint grey (128, 128, 128; or as long as RGB values are pretty much equal; if they are not equal midpoint is going to color the entire image against the prominent chanel of the selected point), and automatically maps based on your selection. If you can't find a blackest point, try a threshold layer and by moving this up and down you can tell the brightest and darkest points. You can also open "info" panel where it tells you the RGB values of a point you are hovering over. Curves eye dropper can save you the work of these two curves section, if you don't need more sophisticated local / single channel adjustments.
* Color correction with **curves**
  * E.g. you have a scanned old photo that looks aged and yellow.
  * To confirm if you want to do color correction, look at individual color channels. If when only one channel is enabled the image looks under-exposed while with others look normal, that channel may need to be dialed up.
  * We do this via individual channel curves adjustment.

### Layer masks



* Curves layer: contrast
* Hue / saturation layer
