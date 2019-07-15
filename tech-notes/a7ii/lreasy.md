# Lightroom made easy

### Introduction

* If were to choose between LR and PS, LR.
* Does not modify original image. It logs your changes and shows you a preview.

* Interface
  * Center: preview area, **g**rid view / loupe view (single image).
  * Top: Task bar (modules: lib, develop, slideshow / print / web: export).
  * Left panel: navigation / templates.
  * Right panel: detailed to this module.
  * Bottom: toolbar (differs by module), film strip.
  * **Shift-Tab**: toggle all panels.
  * **i**: toggle camera info.
  * **l**: lights out, greys out the interface.

### Import, Selection

* Import
  * src, dst, copy as dng / copy / move / add, rename files / rename templates, quick develop during important, blue print / metadata, keywords, import presets
  * don't change folder structure / file name outside Lightroom. If out-of-sync, try synchronizing folder

* Working with your library
  * **x**: set as rejected, **u**: unpick a rejection; filter by rejected, delete from Lightroom / disk
  * flag rating (reject, no mark (keep), favorite / pick **p**)
  * star rating (**1**-**5**; custom meaning, e.g. 1-star => useful mistake, 2-star => keep for parts, 3 => pretty good, 4 => great (cream of the set), 5 => portfolio shot (cream of your career)).
  * tagging photos (keywording), tagging range of photos (**Cmd + D**, deselect range)
  * color labeling
  * survey view (**N**): keeping one among a group of photos by comparing a group side by side, clicking 'x' to take out ones that don't make it
  * compare view: filter one out for specific purposes, e.g. cover shot. O(n) max selection.
  * quick develop: can't tell if a picture's good yet, need quick adjustment (auto, exposure, highlight, black, etc)
  * **Cmd + F**: filter by text / metadata / attribute (e.g. star rating)
  * collections panel: add photos to a collection (pointers to original copies of the photo, not duplicating). rearranging in a collection. Smart collection: add to automatic collection by filter criteria. Collection sets (hierarchical collections)

### Develop

* Develop
  * Non destructive changes. Change history. Editing from history. Create snapshot from point in history.
  * Lightroom presets.
  * **y**: toggle Before/After view.
  * Creating a virtual copy. **Cmd + '**
  * Right develop panel: workflow generally goes top-down
    * 'reset': undo all edits
  * Basic
    * Crop and straightening
      * Grid: by default rule of thirds, cycle through with **o**
      * Optional fixed aspect ratio
      * Staightening ruler: draw a line in your image that should be straight, and rotate accordingly. Manual rotate. Angle slider.
      * Try cropping / straightening in Lights Out mode
    * White balance (do this before you further adjusts your tone)
      * Temperature: Cooler --> Warmer (double click: reset)
      * Tint: Green --> Magenta
      * as-shot / custom / in-camera white balancing profiles (raw only)
      * White-balance eye-dropper: pick a neutral-grey (like concrete) to auto adjust white balancing
    * Tone controls
      * Histogram: distribution of light and dark values.
        * x-axis: Black --> White. Each y-value represents the number of pixels with that grey value.
        * Spilling off the left or right edge means you are missing some detail, if so, the little triangle on the left/right corner of histogram will turn up. Clicking the triangle highlights the pixels spilling off.
        * Different areas in the histogram has different descriptions: blacks, fill light, shadows, exposure, highlights, whites. Hovering over an area highlights the adjustment bar for the area.
      * Phil's usual workflow with raw: adjust blacks down, try the exposure, then shadows (adjusting this up brings out details in shadowy parts), then highlights (recovers details from blown out areas) (to be updated with LR4 / CC)
      * contrast: not Phil's favorite as its function can be achieved with the bars for each area, and more powerful adjustment can be made with tone curve.
      * presence
        * clarity: contrast tool on the midtone (blurry --> edgy, a higher value may not be flattering for a group of people, but may be good for architecture).
        * saturation: the intensity / pureness of colors (turning up means blue becomes very blue)
        * vibrance: LR's smart version of saturation. (turning up it tries not to saturate colors that are already over saturated). Phil almost never uses saturation in LR, as vibrance is a smarter better control.
    * Vignette: to, e.g., keep the focus on your subject
  * Tone curve: for contrast adjustment. Like Curves in Photoshop.
    * Each sub-panel has a "toggle this panel on/off"
  * HSL: per-color hue / saturation / luminance adjustment
    * hue: the color itself. All the colors are mapped to angle values in a 2D circle (or think of it as a single slider selecting those angle values). With per-color hue, you can e.g. make your blue magenta.
    * saturation: the intensity of color as described above.
    * luminance: brightness / darkness of color.
    * there is a one-to-one mapping between HSL and RGB color spaces.
  * Split toning: allows different color hue to highlights / shadows.
  * Details
    * Noise reduction: typically happens with high iso or radical exposure adjustment in post processing. Use the little rectangle to adjust your enlarged preview area.
      * Color noise: colored speckles in an area. (e.g. red and green speckles on a piece of leaf). Could bring up the color slider till speckles of different colors are gone.
      * Luminance noise: grey speckles in an area.
    * There is a tradeoff between noise reduction and detail (sharpening). Could try bringing up details if you are losing edge details, but you'll get some noise back. You don't want to go any further on color / luminance noise bars than you have to as you'll lose details.
    * After you zoom in and adjust these, zoom out to see if it did something strange elsewhere.
  * Sharpening
    * Reduce the noise first as if you don't, you'll end up sharpening the noise in your picture.
    * Capture sharpening: when shooting in jpeg, this is already applied. Shooting in raw doesn't adjust this automatically and you may want to do so yourself.
      * Amount: how much sharpening. Cranking this up too high can sharpen the noise much as well.
      * Radius: how wide a pixel radius to apply sharpening to. 0.8 pixels: image with a lot of fine details; 1.3: less fine details. (An architecture with big hard edges without fine details, can do like 2 pixels)
      * Details: lots of fine details: adjust this up. Big smoother surfaces: adjust this down.
      * Masking: masks parts of image that it thinks you don't want sharpening. (E.g. smooth sky that doesn't want sharpening, vs detailed leaves that should be sharpened. Also, you don't want to sharpen someone's skin, but you might want to sharpen their hair and eyes). Holding **option** while adjusting shows you the area it will sharpen in white. Generally speaking an image with chunks of smooth surfaces and hard edges (a building, an airplane; but not a person / plant) could use more extreme masking and sharpening.
    * Creative sharpening
    * Export sharpening
  * Lens correction
    * Shape of our lenses introduces distortion. The wider-angle the lens, the more distortion. (think fish-eye)
    * Automatatic profile correction (can download / make a profile yourself)
    * Usually when shooting portraits you want to be farther away from subject and use a telephoto lens, as opposed to staying close and use a wide angle lens, as the latter causes your subject's face to bulge towards the camera (noise too large, etc). Manual lens correction can actually adjust this: cranking distortion up creates an effect that makes it seems you are almost shooting with a longer-focal length lens.
  * Effects
    * Post-crop vignette. Most lens comes with a vignette, usually we correct that and add vignetting for a creative effect.
      * Midpoint: thickness
      * Roundness: oval / rectangular vs circular
      * Feather: soft / hard edge
    * Grain: film grain-like effect
  * Camera calibration: e.g. if your camera usually applies too much magenta. Most people don't need to worry about this.

* Tools strip / local adjustment
  * Spot removal
    * Cleaning blemishes off people, removal isolated little objects from simple background, dust on lenses
    * Healing (copy) / cloning mode (merge, e.g. for skin)
    * **h**: hide the little patch indication
  * **Cmd + +/-**: adjust preview size
  * Graduated filter, like a graduated neutral density lens filter
    * E.g. to make a blown out sky less bright and more blue / more dramatic
    * End point gradient has 0-percent effect, starting point 100-percent effect
  * Red eye removal tool
  * Adjustment brush
    * A, B: two brushes.
    * Two concentric rings: size and feathered edge. Controled by size / feather respectively. **(Shift) + \[\]** make bigger or smaller.
    * Flow: like a spray can.
    * Auto mask: tries to keep the effect from leaving a hard edge (areas with large contrast)
    * Density: opacity of the effect.
    * Crank up exposure way up high first and paint so you know where you've painted (or, hovering over the pin shows the area in red / "show selected mask overlay").
    * Enter to accept; otherwise further painting adds area to this adjustment brush. Or if you click "erase" it'll be removing areas from this adjustment brush.

* Portrait retouch
  * Adjustment brush + exposure to whiten the whites of the eyes (+ clarity on the irises to sharpen the eyes)
  * Adjustment brush - exposure clarity for skin softening

* Lightroom and photoshop interaction
  * **Cmd + e** edit in photoshop.
  * Save in photoshop to bring the editted back to Lightroom. Editing that version in Photoshop again (after further LR adjustments, e.g.), keeps the photoshop layers intact.

### Exporting

* File names, export location, pixel dimensions.
* tiff max quality print house requested, usually jpeg (50 - 80). sRGB.
* Resizing pixels. (can enlarge in which case Lightroom extrapolates, usually bad quality). Resolution: only matters for printers. Resizing causes resampling.
* Output sharpening. Unless you plan to do further resharpen in a different program, leave sharpening on.
* Watermark
* Presets
