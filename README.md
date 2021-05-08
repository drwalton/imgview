# imgview
Super-simple image viewer

This is a very simple image viewer based on SDL and OpenGL (using OpenCV to load images).

I mainly made this to have a small, simple alternative to the default image viewer on Windows, but it has a couple of other features which are often useful:
* The 'S' key switches between bilinear and nearest-neighbour interpolation for displaying the image.
* Holding shift whilst pressing left or right to change image will not change your zoom settings (useful when comparing two images of the same size).
* For viewing documents saved as image files, 'T' fits the image to the screen horizontally, and moves to the top. You can then scroll vertically with the right mouse button.

# Installation

On Windows, build the application, put it wherever you like and then you can open image files just by dragging and dropping them onto imgview.exe.

You can also use the "Open With..." context menu option to open a file and/or make imgview the default image viewer for a particular filetype.

# Usage

Once opened you can use the left mouse button to scroll the image, and the mouse wheel to zoom.

Everything else is done with keyboard shortcuts - press 'H' to bring up a list of them.

* Left/Right: Prev/Next Image
* SHIFT+Left/Right: Prev/Next Image (maintain current zoom)
* Mouse drag: Move viewpoint
* Right mouse drag: Move viewpoint vertically only
* Mouse wheel: Zoom in/out
* ALT+ENTER: Toggle Fullscreen
* F: Fit image in window
* T: Fit image in window horizontally, and move to top of image (good for documents)
* SHIFT+F: Resize window to image dimensions
* C: Center image in window
* 0: Set zoom level to 1:1
* S: Switch between linear/nearest neighbour sampling
* B: Toggle transparency (alpha blending)
* K: Cycle through background colors (gray/black/white)
* H: Show help
* Q: Quit application
