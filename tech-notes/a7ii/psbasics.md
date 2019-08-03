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

Many types of masks and different ways to use them.
Layer masks protect parts of the image from changes and expose other parts to the change.

Mask starts out white, meaning they are affected by the change.
* Create a mask by painting black (black foreground color, brush tool **b**) on the mask to say these pixels are now transparent in this layer. Paint white to undo, **x** toggles foreground-background colors.
* Alternatively, select with e.g. magic wand, **shift** to add, **option** to minus. When a selection is active, creating a layer will mask out the unselected areas.
* Holding **option** while dragging a layer mask onto a different layer can create a copy of this mask on the destination layer.
* The cut-out problem. If your selection has a hard edge due to bad selection, try feathering in 'Select-Modify-Feather', this blurs the edge of your selection on either side by the radius you gave.
* to make an image black and white, do a black and white adjustment layer.

### Removing background objects

* Clone stamp tool: sample a part of your image as your paint, to paint it over other parts of your image. **option + click** to sample an area. Like healing brush in LR, but with more control. Paint in small strokes. (don't check "aligned", otherwise sample point stays clocked relative to your paint location). Resample often, soften your brush if painting areas with gradient colors.
* Healing brush tool. **option + click** samples, result is a blur of sample and destination. Sometimes when close to an edge healing will bleed in color over the edge, try clone stamp in affected areas.
* Content-aware fill in current version of photoshop may be better than both options above.
* Selection with quick mask.
  * Selecting an area makes it active (e.g. paint won't affect inactive areas. This makes working around details easier.)
  * Selection mode: replace, add, subtract, intersection. **Cmd + D** to deselect.
  * Lasso selection: free hand. Polygon lasso: freehand polygon. Magnetic lasso: snaps to a high-contrast edge (click to force point, otherwise snaps to an edge and places points automatically). **Delete** to get rid of erroneous points. 
  * Quick mask mode: paint to select. When quick-mask on, highlights selection with color. Paint white to deselect.
  * Save your elaborate selection! ('Select-Save Selection')
  * **Cmd + I** invert selection
  * **Cmd + Option + Shift + E** create a layer by combining all the layers below selected layer. Useful for doing an overall before vs after comparison 
* Each bit of work should go on a different layer.

### Portrait retouching

* Eyebags / shades under the eyes: patch tools, draw a closed circle and drag. replaces source with resampled destination. Consider reducing opacity to make it more natural. Don't overdo it.
* Healing brush wrinkles removal. Take a sample close to the working area to get the colors match. Use a soft edge brush when doing this.
* **Cmd + Alt + Shift + E** after done with each part to merge all underlying into a copy and work on that new copy.
* Bringing out the eyes / teeth whitening: convert blending mode of the new layer to screen, create a black layer mask on the new layer, paint the eyes white, and change opacity. Get a low opacity on this. Alternatively make a selection via painting in smart mask mode, create a hue and saturation adjustment layer, desaturate the yellow channel, don't go all the way down. Bring up the lightness in master channel
* Cloning out blemishes in skin: clone stamp tool. Bring down opacity to smoothen skin, or full opacity to one click remove a blemish, or take out makeup edges
* Skin softening: duplicate the merged layer twice. turn the lower one blend mode to lighten, apply gaussian blur (radius 25) (50 opacity); turn the upper one blend mode to darken, apply gaussian blur (radius 15) (50 - 75 opacity), make only these half opaque layers visible, merge them into one layer, adjust the opacity of that merged layer until the skin looks natural, then cut through the merged layer to reveal the sharp details. 100 opacity paint black to cut out the eyes, eye brows and teeth, but consider reducing the opacity when cutting out nose / skin edges.
* If you are concerned about disk space, flatten the image before saving it. This loses all the individual steps / layers.

### Cropping, resizing, and resampling

* Crop tool (**C**), crop with width and height set
* Straightening horizon: ruler tool, drag a supposedly straight line. Then Image->Rotate->Arbitrary. This preloads the angle of the ruler's drawn line. Or show grid, and rotate the photo to line up with a gridline: select all and free transform
* Correcting distorted perspective (e.g. shooting a building from beneath it: with crop, check perspective, adjust the sides such that they are parellel with the edge of the building, then crop.
  * In PS CC, use Perspective Crop tool, make a trapezoid shape with sides parallel to the building's edges.
* Resizing an image by resampling. When you resample, you lose quality; both up sampling (interpolation) and down sampling (averaging).
  * After you resample, sharpen your photo

### Sharpening

* Filter->Sharpen->Unsharp Mask (pretty subtle)
  * amount: how much to sharpen, typically between 50 - 150 percent, the higher the value, the more sharpening will be applied.
  * radius: how far out around any pixel the sharpening apply, typically 1 - 2 pixels. the higher the value, the more sharpening will be applied. This also has to do with the number of pixels in the original image.
  * threshold: how different the two pixels side by side has to be, before that is considered an edge (sharpening algorithm tries to find the edges and increase the contrast of the edges). typically 4 - 10. the higher the value, the less sharpening will be applied.
* View your image at 100% while sharpening. Things with hard edges / are geometrical could use more sharpening. Soft (think natural, feathered animals) / hard objects (think man-made, buildings).
* Luminosity sharpening: sharpen your lights but not your colors. If you see sharpening bringing in color aberation, you might want to try luminosity sharpening to get rid of color aberation. After you apply your sharpening, fade unsharp mask, change mode to luminosity, and ok changes the sharpening you just did to apply only to luminosity.
* You don't want to sharpen a person's skin. Do masking if you have time, if not and you still want to sharpen eyes, etc, sharpen just the channel that doesn't have too much details (in a person's case, often times red.)

* Curves layer: contrast
* Hue / saturation layer
