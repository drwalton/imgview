#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "ImgviewApp.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

const int verticalDisplayOffset = 75; // When the window is created, this offset is used to move the window down
// slightly so that the title bar is visible. This value was set for windows 10, you may wish to change it on other
// platforms.

#ifdef _MSC_VER
std::wstring ToUtf16(std::string str)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &ret[0], len);
	}
	return ret;
}
#endif


const std::vector<std::string> supportedExtensions = {
	".png", ".jpg", ".jpeg",// ".gif",
	".PNG", ".JPG", ".JPEG",//, ".GIF"
	".webp", ".WEBP",
	".tif", ".tiff", ".TIF", ".TIFF",
	".pgm", ".PGM",
	".bmp", ".BMP"
	//".exr", ".EXR"
};

const int initWidth = 1000, initHeight = 1000;
float offset[] = {0.f, -initHeight};
const float multisampleFactor = 2.f;
const float minZoom = 0.01f;
#ifdef __APPLE__
const float scrollSpeed = 0.01f;
#else
const float scrollSpeed = 0.1f;
#endif
const size_t nClearColors = 3;

GLuint compileShader(const std::string &source, GLenum type);
GLuint createShaderProgram(GLuint vertShader, GLuint fragShader);
void setupVertexBuffers(GLuint program);

ImgviewApp::ImgviewApp(const std::string &filename)
	:redraw_(true),
	running_(true),
	mouseDown_(false),
	rightMouseDown_(false),
	zoom_(1.0f),
	winAspect_((float)(initWidth) / (float)(initHeight)),
	winWidth_(initWidth), winHeight_(initHeight),
	isFullscreen_(false),
	context_(0, 30, initWidth, initHeight),
	samplingMode_(SamplingMode::LINEAR),
	clearColorIdx_(0),
	alphaBlendEnabled_(true),
	imagePathIdx_(0),
	currRotation_(0)
{
	glViewport(0, 0, initWidth, initHeight);
    // Create and install global locale
	boost::locale::generator gen;
	gen.use_ansi_encoding(true);
    std::locale::global(gen.generate(""));
    // Make boost.filesystem use it
    boost::filesystem::path::imbue(std::locale());
    // Now Works perfectly fine with UTF-8!

	initialImagePath_ = filename;
	if(!boost::filesystem::is_regular_file(initialImagePath_)) {
		throw std::runtime_error(
			"Supplied image is not present, or not a regular file");
	}

	loadImagePaths();
	loadImage(filename);
	fitImageToWindow();
	
	setTitle();

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	int height = DM.h - verticalDisplayOffset; int width = DM.w / 2;
	SDL_SetWindowSize(context_.window(), width, height);
	handleResize(width, height);
}

ImgviewApp::~ImgviewApp() throw()
{
	if(texture_ != 0) {
		glDeleteTextures(1, &texture_);
	}
}

void ImgviewApp::run()
{
	SDL_Event event;

	while(running_) {
		SDL_WaitEvent(&event);
		processEvent(event);
		if(redraw_) {
			redrawImage();
			redraw_ = false;
		}
	}
}

void ImgviewApp::loadImagePaths()
{
	// Find all files in current directory with an extension in the supported extensions.
	imagePaths_.clear();
	boost::filesystem::path imDir = initialImagePath_.parent_path();
	boost::filesystem::directory_iterator i(imDir);
	boost::filesystem::directory_iterator endItr;
	for(; i != endItr; ++i)
	{
		if(boost::filesystem::is_regular_file(*i)) {
			if(supportedFileExtension(i->path().extension().string())) {
				imagePaths_.push_back(i->path().string());
				if(*i == initialImagePath_) {
					imagePathIdx_ = imagePaths_.size() - 1;
				}
			}
		}
	}
	if(imagePaths_.size() == 0) {
		throw std::runtime_error("No images left in directory");
	}

	// Sort image paths to get correct ordering.
	std::sort(imagePaths_.begin(), imagePaths_.end());

	// Find index of initial loaded image so this is correct after sorting.
	auto idx = std::find(imagePaths_.begin(), imagePaths_.end(), initialImagePath_.string());
	if (idx == imagePaths_.end()) {
		// Couldn't find the index, so set it to first image in directory as failsafe.
		imagePathIdx_ = 0;
	}
	else {
		imagePathIdx_ = std::distance(imagePaths_.begin(), idx);
	}
}

void ImgviewApp::redrawImage()
{
	if (clearColorIdx_ == 0) { // gray
		glClearColor(0.4f, 0.4f, 0.4f, 1.f);
		shaderProgram_.setBgColor(0.4f, 0.4f, 0.4f, 1.f);
	}
	else if (clearColorIdx_ == 1) { // black
		glClearColor(0.f, 0.f, 0.f, 1.f);
		shaderProgram_.setBgColor(0.f, 0.f, 0.f, 1.f);
	}
	else if (clearColorIdx_ == 2) { // white
		glClearColor(1.f, 1.f, 1.f, 1.f);
		shaderProgram_.setBgColor(1.f, 1.f, 1.f, 1.f);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	shaderProgram_.use();
	shaderProgram_.bindVertexArray();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	if (alphaBlendEnabled_) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else {
		glDisable(GL_BLEND);
	}
	glDrawArrays(GL_TRIANGLES, 0, 6);
	shaderProgram_.unbindVertexArray();
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_GL_SwapWindow(context_.window());
}

const std::string controlsString =
"Left/Right: Prev/Next Image\n"
"SHIFT+Left/Right: Prev/Next Image (maintain current zoom)\n"
"Mouse drag: Move viewpoint\n"
"Right mouse drag: Move viewpoint vertically only\n"
"Mouse wheel: Zoom in/out\n"
"ALT+ENTER: Toggle Fullscreen\n"
"1-9: Move window to display #\n"
"F: Fit image in window\n"
"SHIFT+F: Resize window to image dimensions\n"
"R: Rotate image 90 degrees clockwise\n"
"T: Fit image horizontally, and move to top of image\n"
"C: Center image in window\n"
"0: Set zoom level to 1:1\n"
"S: Switch between linear/nearest neighbour sampling\n"
"K: Cycle through different background colors\n"
"B: Toggle alpha blending (transparency)\n"
"H: Show these controls\n"
"Q: Quit application\n";

void ImgviewApp::updateSamplingMode()
{
	glBindTexture(GL_TEXTURE_2D, texture_);
	if (samplingMode_ == SamplingMode::LINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else if (samplingMode_ == SamplingMode::NEAREST) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	redraw_ = true;
}

void ImgviewApp::processEvent(SDL_Event &event)
{
	if(event.type == SDL_QUIT ||
		 (event.type == SDL_WINDOWEVENT &&
		event.window.event == SDL_WINDOWEVENT_CLOSE)) {
			 running_ = false;
		 }

	if(event.type == SDL_MOUSEBUTTONDOWN) {
		if (event.button.button == SDL_BUTTON_LEFT) {
			mouseDown_ = true;
		}
		else if (event.button.button == SDL_BUTTON_RIGHT) {
			rightMouseDown_ = true;
		}
	}
	if(event.type == SDL_MOUSEBUTTONUP) {
		if (event.button.button == SDL_BUTTON_LEFT) {
			mouseDown_ = false;
		}
		else if (event.button.button == SDL_BUTTON_RIGHT) {
			rightMouseDown_ = false;
		}
	}

	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RETURN) {
			if (event.key.keysym.mod & KMOD_ALT) {
				if (isFullscreen_) setFullscreen(false);
				else setFullscreen(true);
			}
		}

		if (event.key.keysym.sym == SDLK_q) {
			running_ = false;
		}
		if (event.key.keysym.sym == SDLK_c) {
			centerImageInWindow();
		}
		if (event.key.keysym.sym == SDLK_0) {
			resetZoom();
		}
		const SDL_KeyCode displayKeys[] = {SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9};
		for (int i = 0; i < 9; ++i) {
			if (event.key.keysym.sym == displayKeys[i]) moveToDisplay(i);
		}

		if (event.key.keysym.sym == SDLK_h) {
#ifdef _MSC_VER
			MessageBox(NULL, controlsString.c_str(), "imgview Controls", MB_OK);
#endif
		}
		if (event.key.keysym.sym == SDLK_s) {
			if (samplingMode_ == SamplingMode::LINEAR) {
				samplingMode_ = SamplingMode::NEAREST;
			} else if (samplingMode_ == SamplingMode::NEAREST) {
				samplingMode_ = SamplingMode::LINEAR;
			}
			updateSamplingMode();
		}
		if (event.key.keysym.sym == SDLK_f) {
			if (!(event.key.keysym.mod & KMOD_LSHIFT)) {
				fitImageToWindow();
			} else {
				fitWindowToImage();
			}
		}
		if (event.key.keysym.sym == SDLK_t) {
			fitImageToWindowHorz();
		}
		if (event.key.keysym.sym == SDLK_k) {
			++clearColorIdx_;
			clearColorIdx_ %= nClearColors;
			redraw_ = true;
		}
		if (event.key.keysym.sym == SDLK_b) {
			alphaBlendEnabled_ = !alphaBlendEnabled_;
			redraw_ = true;
		}
		if (event.key.keysym.sym == SDLK_r) {
			currRotation_ += 1;
			redraw_ = true;
			loadImage(imagePaths_[imagePathIdx_], currRotation_);
			fitImageToWindow();
		}

		if (event.key.keysym.sym == SDLK_RIGHT) {
			++imagePathIdx_; imagePathIdx_ %= imagePaths_.size();
		}
		if (event.key.keysym.sym == SDLK_LEFT) {
			imagePathIdx_ = imagePathIdx_ == 0 ? imagePaths_.size() - 1 : imagePathIdx_ - 1;
		}
		if (event.key.keysym.sym == SDLK_LEFT ||
			event.key.keysym.sym == SDLK_RIGHT) {
			currRotation_ = 0;
			if (!loadImage(imagePaths_[imagePathIdx_])) {
				//Couldn't load this image. Try rescanning directory.
				std::cout << "Couldn't load image "
					<< imagePaths_[imagePathIdx_]
					<< " rescanning directory..." << std::endl;
				size_t tmp = imagePathIdx_;
				loadImagePaths();
				imagePathIdx_ = tmp % imagePaths_.size();

				if (!loadImage(imagePaths_[imagePathIdx_])) {
					//After rescanning directory, still couldn't load image.
					throw std::runtime_error("Couldn't open image " +
						imagePaths_[imagePathIdx_]);
				}
			}
			setTitle();
			if (!(event.key.keysym.mod & KMOD_LSHIFT)) {
				//Left shift wasn't held down
				handleResize(winWidth_, winHeight_);
				fitImageToWindow();
			} else {
				redraw_ = true;
			}
			updateZoom();
		}
	}
	if (event.type == SDL_WINDOWEVENT &&
		event.window.event == SDL_WINDOWEVENT_RESIZED) {
		handleResize(event.window.data1, event.window.data2);

	}
	if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y != 0) {
			float newZoom = 1.f / zoom_ + scrollSpeed * event.wheel.y;
			newZoom = newZoom > minZoom ? 1.f / newZoom : 1.f / minZoom;
			float prevZoom = zoom_;
			zoom_ = newZoom;

			// This adjusts the offset to keep the point in the image that's currently under the mouse cursor 
			// in the same window location at the new zoom level.
			offset[0] = (prevZoom / zoom_) * (mouseX_ + offset[0]) - mouseX_;
			float y = winHeight_ - mouseY_; // Need to flip y as GL uses +ve y up
			offset[1] = (prevZoom / zoom_) * (y + offset[1]) - y;
			shaderProgram_.setOffset(offset[0], offset[1]);
			updateZoom();
			setTitle();
			redraw_ = true;
		}
	}
	if (event.type == SDL_MOUSEMOTION) {
		mouseX_ = event.motion.x;
		mouseY_ = event.motion.y;
		if (mouseDown_ || rightMouseDown_) {
			if (mouseDown_) {
				offset[0] -= event.motion.xrel;
			}
			offset[1] += event.motion.yrel;
			shaderProgram_.setOffset(offset[0], offset[1]);
			redraw_ = true;
		}
	}
}


bool ImgviewApp::loadImage(const std::string& filename, int rotation)
{
	// On windows, convert to wstring as only utf16 is supported.
#ifdef _MSC_VER
	std::wstring str = ToUtf16(filename);
#else
	std::string str = filename;
#endif

	//Imread doesn't handle Unicode, so need to read into an ifstream and then use imdecode
	std::ifstream f(str, std::iostream::binary);

	// Get its size
	std::filebuf* pbuf = f.rdbuf();
	size_t size = pbuf->pubseekoff(0, f.end);
	pbuf->pubseekpos(0, f.in);
	if (size == 0 || size == -1) {
		return false;
	}

	// Put it in a vector
	std::vector<uchar> buffer(size);
	pbuf->sgetn((char*)buffer.data(), size);

	// Decode the vector
	cv::Mat mat = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
	if (mat.rows == 0 || mat.cols == 0) {
		return false;
	}

	//std::cout << "Loaded image " << filename << " " << mat.cols << "x"
		//<< mat.rows << ", " << mat.channels() << " channels" << std::endl;

	if (mat.channels() != 3 && mat.channels() != 4) {
		mat = cv::imdecode(buffer, cv::IMREAD_COLOR);
	}
	if (mat.channels() == 3) {
		imFormat_ = "RGB";
		cv::cvtColor(mat, mat, cv::COLOR_BGR2BGRA);
	}
	else {
		imFormat_ = "RGBA";
	}
	rotation = rotation % 4;
	for (int r = 0; r < rotation; ++r) {
		cv::rotate(mat, mat, cv::ROTATE_90_CLOCKWISE);
	}

	imAspect_ = (float)(mat.cols) / (float)(mat.rows);
	imWidth_ = mat.cols; imHeight_ = mat.rows;

	if(texture_ != 0) {
		glDeleteTextures(1, &texture_);
	}

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	if (samplingMode_ == SamplingMode::LINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else if (samplingMode_ == SamplingMode::NEAREST) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0,
				 GL_RGBA8, mat.cols, mat.rows, 0,
				 GL_BGRA, GL_UNSIGNED_BYTE, mat.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void ImgviewApp::handleResize(int newWidth, int newHeight)
{
	updateZoom();
	glViewport(0, 0, newWidth, newHeight);
	winAspect_ = float(newWidth) / float(newHeight);
	redraw_ = true;
	winWidth_ = newWidth;
	winHeight_ = newHeight;
	fitImageToWindow();
}

void ImgviewApp::fitImageToWindow()
{
	if(imAspect_ < winAspect_) {
		//Fit vertically
		zoom_ = float(imHeight_) / float(winHeight_);
	} else {
		//Fit horizontally
		zoom_ = float(imWidth_) / float(winWidth_);
	}
	updateZoom();
	centerImageInWindow();
}

void ImgviewApp::fitImageToWindowHorz()
{
	zoom_ = float(imWidth_) / float(winWidth_);
	updateZoom();
	centerImageInWindowTop();
}

void ImgviewApp::fitWindowToImage()
{
	SDL_SetWindowSize(context_.window(), imWidth_, imHeight_);
	handleResize(imWidth_, imHeight_);
}

void ImgviewApp::centerImageInWindow()
{
	offset[0] = -((winWidth_*.5f) - (imWidth_/(multisampleFactor*1.f*zoom_)));
	offset[1] = -winHeight_*.5f - (imHeight_/(multisampleFactor*1.f*zoom_));
	shaderProgram_.setOffset(offset[0], offset[1]);
	redraw_ = true;
}

void ImgviewApp::centerImageInWindowTop()
{
	offset[0] = -((winWidth_*.5f) - (imWidth_/(multisampleFactor*1.f*zoom_)));
	offset[1] = static_cast<float>(-winHeight_);
	shaderProgram_.setOffset(offset[0], offset[1]);
	redraw_ = true;
}

void ImgviewApp::setFullscreen(bool fullscreen)
{
	if (fullscreen) SDL_SetWindowFullscreen(context_.window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
	else SDL_SetWindowFullscreen(context_.window(), 0);
	isFullscreen_ = fullscreen;
}

void ImgviewApp::moveToDisplay(int displayIndex)
{
	bool wasFullscreen = isFullscreen_;
	if (isFullscreen_) setFullscreen(false);

	int numDisplays = SDL_GetNumVideoDisplays();
	if (displayIndex >= numDisplays) return; // If the display selected isn't connected, just return early.
	
	// Get dimensions of the selected display in the global display space.
	SDL_Rect displayBounds;
	SDL_GetDisplayBounds(displayIndex, &displayBounds);

	SDL_SetWindowPosition(context_.window(), displayBounds.x, displayBounds.y + verticalDisplayOffset);

	if (wasFullscreen) setFullscreen(true);
}

void ImgviewApp::resetZoom()
{
	zoom_ = 1.f;
	updateZoom();
	redraw_ = true;
}

void ImgviewApp::updateZoom()
{
	shaderProgram_.setFrag2Tex(.5f*zoom_*multisampleFactor/imWidth_,
		-.5f*zoom_*multisampleFactor/imHeight_);
	setTitle();
}

bool ImgviewApp::supportedFileExtension(const std::string &extension)
{
	for(auto &e : supportedExtensions) {
		if (e == extension) {
			return true;
		}
	}
	return false;
}

void ImgviewApp::setTitle()
{
	std::stringstream title;
	title << "imgview: " << boost::filesystem::path(imagePaths_[imagePathIdx_]).filename().string()
		<< " (" << imagePathIdx_ << "/" << imagePaths_.size() << ") "
		<< imFormat_ << " " << imWidth_ << "x" << imHeight_ << "@" << 100.f / zoom_ << "%";
	SDL_SetWindowTitle(context_.window(), title.str().c_str());
}

