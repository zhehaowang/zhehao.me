# Controls in A7 II

### Camera modes

* Auto
* Program mode (P). Auto.
* Aperture Priority (A). Front dial controls aperture (camera controls shutter, iso)
  * Analogous to pupil.
  * Measured by. f/x, the smaller x, the bigger aperture (more light is let in), the more background blur (see depth-of-field). (e.g. f/1.8 - f/22). f/2.8 lets in 4 times as much light as f/5.6. Typically each f stop steps are arranged by a factor of `sqrt(2)`, meaning each step brings in double the amount of light, and your shutter speed can get cut by half, while iso remains unchanged to get the same exposure. A f/3.5-f/5.6 on a zoom lens says at minimum / maximum focal lengths, what the lowest f stop numbers are.
  * Userful for controlling background blur. (as a lower f stop number gives you shallower DoF)
* Shutter Priority (S). Front dial controls shutter (camera controls aperture, iso)
  * How long the shutter is open for. You can shoot in smallest shutter period possible, in which case camera will likely dial up iso, resulting in noises. To capture fast moving objects you generally want faster shutter speed.
  * Measured by seconds. E.g. 1/60s.
  * Useful for capturing movement: think waterfall / clouds if you want it to look feathery and soft; think cars, you might want to show wheels spinning; musicians, you might want to show hand / drumsticks movement while keeping the face in focus; think a busy street with a crowd at night, or a big crowd at a monument that you want to blur away; or light painting.
  * Distinguish your desirable motion blur with undesirable ones of the subject; distinguish camera shake with motion blur (background moves or not)
  * Reciprocal rule, shutter speed at 1/(your focal length), depends on the individual and image stabilization. May not be applicable for modern technology.
  * Too fast and you may have flash sync problem. Flash does not fully fire at your high shutter speed.
  * Experiment. In general portraits / still animals 1/30 - 1/250; soaring birds 1/1000 - 1/2000; kids sports 1/125 - 1/250; adult sports 1/500 - 1/1000; star trails 5mins+ (usually the longer the better) etc
* Manual mode (M). Front dial controls aperture. Back dial controls shutter speed. (camera controls iso by default, unless you turn that off as well.)
  * Can switch to bulb mode, where shutter can open for more than 30s by holding. (E.g. night sky)
  * When you need to control background blur and freeze movement.
  * Manual choice isn't the only choice for professionals (even when they do use manual it's often combined with auto iso thus auto-exposure), as they always have more important properties to worry about and maybe a limited time frame. However manual mode is the right choice for studio work.
  * If you are shooting manual, consider following these steps: choose the slowest shutter speed that freezes motion and captures camera shake (or represents motion), choose the lowest f/stop number that gets your entire subject sharp and within depth-of-field, and select the iso you need (or let camera select iso). (slowest shutter and lowest f stop gets you as much light so you'll be minimizing iso to reduce noise). Camera auto works in similar ways: in shutter priority mode, it will try to use the lowest f/stop unless there is already too much light; and in aperture priority mode, it will try to use the slowest shutter speed that will cancel camera shake without going below about 1/60 (again, unless there is already too much light).
* Video mode.

### Drive mode (shutter mode)

* Single shooting
* Continuous shooting
* Self timer setting

### Bracketing (HDR)

* E.g. 2ev 3img, takes 3 pictures at -2, 0, +2 (controls exposure, brighter or darker)
* Useful for HDR (high dynamic range), where you blend different exposures and take the best of each
* This camera has HDR built-in, but it may not do a good job

### Focusing mode

* Single-shot auto focus (AFS): once you focus (hold shoot half down), lock in focus even if you move the camera. Allows focus recompose (most cameras auto focus best the centered object, you could use AF on that, then shift the camera so that the subject is in the thirds. Watch out if you have a very shallow DoF. You will be rotating the camera so with a shallow DoF this may cause your subject to be out of focus).
* Continuous auto focus (AFC): as the subject moves, refocus continuously. Not especially reliable. Can use for action. A lower f stop lens also makes it faster to autofocus moving subjects.
* Dynamic manual focus (DMF): auto focus by default, but you can also grab the focus ring (lens) and focus manually.
* Manual focus (MF): disable auto focus. Disable auto focus. Does nothing when shoot half down.

Cameras autofocus by focusing farther and closer until they find maximum contrast.
If you are attempting to focus on a solid color, the autofocus system won't be able to find any contrast, regardless of the amount of light you have.

Try using a single auto focus point.

### Focus area

* Wide: camera picks focus point for you
* Flexible spot: always focus on the area you choose
  * The smaller focus area, the easier to pin point something especially with a shallow depth of field, the longer the focus time
* Continuous focus
* Zone focus: e.g. left third
* Center focus

### Manual focus

* Mirrorless has these features making manual focus more practical. On DSLRs, manual focus using a small viewfinder is less practical than autofocus, manual focusing and zooming in with live view takes time.
  * Manual focus assist: when Manual Focus is on, grabbing focus ring zooms in in monitor
  * Focus peaking: colors what's in focus

### Adapting lenses

* APS-C size capture, auto lets the camera decides if the attached lens is APS-C, otherwise force full-frame
* Steady shots, if you attach a non-Sony lens, may need to set steady shots focal lens to manual to match your lens.

### ISO

Sensitivty of your sensor. Given a certain shutter speed and aperture, low iso gives dark image, high iso gives bright image.
* Auto allows camera to choose an iso that properly exposes the picture.
* Manual, 12800, ..., 100, 50; higher iso has higher noise (especially in shadows). Try to get your camera at base iso, 100 / 200. You should always use the lowest iso that will allow you to stop motion blur and camera shake. Noise reduction software such as lightroom can help with reducing noise.
* The human eyes can also adjust iso: if you spend some time in a dark room, your body increases rhodopsin level in your retina, allowing your eyes to be more sensitive to light. (human eyes can do 50-30K iso)
* Cameras have a limited dynamic range compared with eyes. If your camera has a **dynamic range** of 10 **stops** (exponentials of 2), that means you'll be able to see subjects in the shadows and highlights of a picture as long as the brightest parts aren't more than 1000 times brighter than the darkest parts. Professional cameras usually have a dynamic range of 8-12 stops, while human eyes can do about 20 stops. That's why exposure's a challenge: your eyes can see the underexposed part because it's less than 2^20 times darker than the sky, but your camera can't because it's more than 2^10 darker.
* Because our eyes and brain automatically adjust to different light levels, your living room seems to be about as bright in the day lit by sunlight as it is at night lit by artificial lights. Your camera will teach you it's actually about 10 times brighter at day.
* Exposure compensation can adjust the overall exposure of the picture, A7ii offers -3 to +3 stops. You should always prefer exposure compensation to adjusting exposure in post processing, as exposure compensation can affect the camera's automatic decisions (be it aperture, iso, or shutter speed, unless you shoot in full manual mode), leaving you with a different raw file to work with in post processing, one that is usually better suited to your judgment of the scene. (No matter what you do later, you cannot get what was outside of the captured dynamic range back.)
* Blinkies (exposure highlight warnings), camera's warning when parts of your image is overexposed.

### (Luminence) histogram

Black-white distribution on the spectrum of shadow - highlight, e.g. if parts of your image is dark, you'll see a spike in the left of the histogram.

In general, try avoiding spikes unless, say, you intend to create silhouettes, or are shooting in the snow.
A properly exposed picture has a histogram that usually does not peak on the very left or the very right end (a digital camera will not be able to recover details from something fully black or fully white; but know that the histogram is based on the jpeg, not the raw file. (the editting program does, though, when you import raw) When your jpeg histogram says something is fully blown out, it may not be so in raw).
The distribution of the histogram generally does not make or break a shot: it's a tool for you to understand if the exposure is as you expected.
E.g., when shooting portraits e.g., try to get your subject to peak in the center. And when your histogram does not touch either end and peaks very much in the middle, it's probably washed out. A U-shaped image is high contrast. Spike in midtone with nothing on the right side means it's probably underexposed, consider adding exposure compensation. Highlights are usually the sweetest parts of your picture: it has much less noise than the shadows.

Some scenes can be too dark and too bright at the same time: histogram peaking in both ends. Exposure compensation wouldn't be able to help in this case. Try fill flash (if the subject is going to be under exposed), or HDR. Often times the sky in daylight is going to be overexposed, polarizing filters help darken the sky in this case, too.

Always look at the histogram, don't trust just the LCD display.

Color histogram: can help you decide if white balancing is off.

_Is shutter speed reflected in the luminence histogram on camera?_

### Metering modes

Decides how the camera should auto adjust iso, etc
* Multi: default. Usually intelligent.
* Center-weighted (matrix / evaluative): choose exposure by averaging the brightness of the entire picture, typically subject in the center is given more weight.
* Spot: choose exposure by using the brightness at a very small area in either the center or the focus point. This is rarely the right choice for digital cameras.

### Diopter

* Next to EVF, for glasses users looking through EVF.

### Shooting raw

* You can shoot raw or jpeg
* Choose in quality: raw, fine, jpeg, raw & jpeg
* Picasa, or Lightroom can work with raw
* Shooting raw allows to
  * recover details from highlights and shadows
  * brighten or darken the picture
  * adjust color temperature
  * reduce noise
  * customize sharpening settings

### Other settings

* Customize keys, customize
  * control wheel
  * buttons C1, C2, etc
  * which button focuses (back-button focus e.g.)
* Audio signals off (no focusing beeps)

### Tripods

# Concepts

### Depth of field

The range of distance in an image where the focus is acceptably sharp.
Lens focal length, aperture, and distance to your subject decides DoF.

* Short lens focal length (zoom out): deep DoF, wide field of view, objects appear farther away from your subject. Long lens focal length (zoom in): shallow DoF, narrow field of view, objects appear closer to your subject
* Aperture: the wider the aperture (lower f stop number), the shallower DoF
* Distance to subject: the closer you are to your subject, the shallower DoF
* Sensor sizes: the larger the sensor, the shallower DoF (a 100mm f/2.8 lens on a cropped sensor by crop factor 2 generates the same perspective and DoF as a 200mm f/5.6 lens on a full frame lens)

DoF can be used to
* isolate your subject from other elements in a photo (e.g. portrait background blur),
* foreground blur, remove obstacles (e.g. fences),
* give better bokeh (visual quality of out-of-focus parts of an image: e.g. smooth circular light spots from an out-of-focus background)

### Lenses

Our eyes are used to seeing the world at around 50mm focal length, phone cameras capture images around 30mm.
To go below that with a short focal length wide-angle or a telephoto focal length can often reflect the world in ways our eyes were not used to seeing.
Wide-angle lens may make it harder to compose the image, a lot will be in your field of view, and you have a larger depth of field.
With a telephoto lens you are going to remove a lot of the context, e.g. creating an abstract image. Telephoto lens also pull distant things together.

Similarly, we are used to seeing the world 1.2m to 1.7m above the ground. When you get the camera lower or higher than that, it could also reflect the world in ways our eyes were not used to seeing.

If you are out and don't have the lens of the desired focal length with you, think differently and compose differently, move around, experiment.

### Misc

* In landscape photography, travel as light as possible: anything that dissuades you from moving is going to hurt, and your creativity / focus will be restricted if you are out of breath.
* iPhone 10 portrait mode (shallow DoF effect): rear camera portrait mode uses two lenses (a zoom lens and a telephoto lens right next to each other, to capture depth information) to take two pictures at the same time and then blend them. Front camera portrait mode uses an infrared sensor to build point cloud to infer depth information.
Say you are wearing glasses / hat and looking sideways, because of the non-optimal nature of the iPhone's blend, it may have trouble keeping the edge lines of your glasses / hat sharp. 


### Sharpness

### Mirrorless vs DSLR

Size, electronic / optical view finder, focus time, battery life, etc

### Tripod

* Head (pan \& zoom (two degree-of-freedom), three lever, ball head)
* Feet (replaceable, e.g. nails)
* Legs (flipped, twisted)
* Material (aluminum, carbon fiber)
* Center column (gets wobbly, raise it only you need to)

Look for one tall enough

